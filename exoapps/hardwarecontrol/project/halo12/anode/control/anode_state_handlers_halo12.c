/*
           Copyright (C) 2022 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file via any medium is strictly prohibited.
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this 
 software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#include "anode_adc.h"
#include "anode_event_handlers.h"
#include "anode_mcu.h"
#include "anode_pwm.h"
#include "anode_state_handlers.h"
#include "component_communication.h"

#define SPARK_TIMEOUT_THRESHOLD       5000 // Trips through the control loop
#define STEADY_STATE_THRESHOLD        50   // Trips through the control loop
#define CURRENT_LOW_THRESHOLD         100  // Counts
#define LOW_CURRENT_TIMEOUT           25   // Control cycles before low current error
#define CONTROL_SATURATION_HYSTERESIS 10   // Counts

#define START_WITHOUT_SPARK_THRESHOLD 0.5 // Threshold to say the anode started
                                          // even if no spark interrupt was triggered
                                          // in A
#define VOLTAGE_IMBALANCE_THRESHOLD   2   // Number of voltage imbalances needed to
                                          // trigger an error

static uint8_t low_current_counter = 0;
static uint16_t voltage_command = 0;
static uint16_t run_count = 0;
static bool control_saturated = false;
static uint8_t voltage_imbalance_counter = 0;

void ash_reset_counters()
{
    low_current_counter = 0;
    voltage_command = 0;
    run_count = 0;
    control_saturated = false;
}

static void mode_select(void)
{
    // change modes if the hysteresis threshold has been passed
    if(anode.target_voltage > (2.1 * (anode.raw_input_voltage * ANODE_INPUT_TO_OUTPUT)))
    {
        if(anode.x_pwm_output == anode.y_pwm_output)
        {
            // this is the standard situation where target voltage is more than twice the input voltage
            // x and y PWMs always track each other in this situation
            anode.mode = QUADRATIC_BOOST;
        } else {
            // this is a transition situation where a new higher target has been received, but the x and y PWMs are not in sync
            // x and y PWMs will be brought together before control is resumed
            anode.mode = TRANSITION_TO_QUAD_BOOST;
        }
    }
    else if(anode.target_voltage < (1.9 * (anode.raw_input_voltage * ANODE_INPUT_TO_OUTPUT)))
    {
        if(anode.x_pwm_output == 0)
        {
            // this is a situation where target voltage is less than twice the input voltage, so the supply acts as a single boost
            // x is off and y is doing all the boosting
            anode.mode = SINGLE_BOOST;
        } else {
            // this is a transition situation where a new lower target has been received, but x is still on
            // x will be turned off before control is resumed
            anode.mode = TRANSITION_TO_SINGLE_BOOST;
        }
    }

    // if the hysteresis threshold has not been passed but a transition is complete, change modes
    else if(anode.mode == TRANSITION_TO_QUAD_BOOST && anode.x_pwm_output == anode.y_pwm_output)
    {
        anode.mode = QUADRATIC_BOOST;
    }
    else if(anode.mode == TRANSITION_TO_SINGLE_BOOST && anode.x_pwm_output == 0)
    {
        anode.mode = SINGLE_BOOST;
    }
}

static void power_check_startup_state(void) 
{
    run_count++;
    
    if (anode.output_voltage > ANODE_OVER_VOLTAGE_COUNTS)
    { 
        anode.common.error_adc = anode.output_voltage;
        anode.common.error_code = ANODE_OVER_VOLTAGE_ERROR;
    }
    else if (run_count > SPARK_TIMEOUT_THRESHOLD)
    {
        anode.common.error_code = ANODE_SPARK_TIMEOUT_ERROR;
    }
    
    if(anode.common.error_code != ANODE_NO_ERROR)
    {
        anode.common.current_state = anode_error_handler();
    } 
    // Start without spark check
    else if((anode.output_current > (START_WITHOUT_SPARK_THRESHOLD * ANODE_COUNTS_PER_AMPERE)) &&
            (anode.common.current_state == STARTUP_STATE))
    {
        anode.common.current_state = anode_spark_detected_handler();
    }
}

static void power_check_on_state(void) 
{
    if (anode.output_voltage > ANODE_OVER_VOLTAGE_COUNTS)
    { 
        anode.common.error_adc = anode.output_voltage;
        anode.common.error_code = ANODE_OVER_VOLTAGE_ERROR;
    }
    else if (anode.output_voltage * anode.output_current > ANODE_MAX_POWER_COUNTS_SQR)
    {
        anode.common.error_adc = anode.output_voltage * anode.output_current;
        anode.common.error_code = ANODE_OVER_POWER_ERROR;
    }
    else if (anode.mode == SINGLE_BOOST && (anode.output_voltage * anode.output_current) > ANODE_MAX_POWER_SINGLE_BOOST_COUNTS_SQR)
    {
        anode.common.error_adc = anode.output_voltage * anode.output_current;
        anode.common.error_code = ANODE_OVER_POWER_ERROR;
    }
    else if (anode.output_current < CURRENT_LOW_THRESHOLD)
    {
        low_current_counter++;
        if(low_current_counter > LOW_CURRENT_TIMEOUT)
        {
            anode.common.error_adc = anode.output_current;
            anode.common.error_code = ANODE_UNDER_CURRENT_ERROR;
        }
    } else low_current_counter = 0;
    
    run_count++;

    if(run_count == STEADY_STATE_THRESHOLD)
    {
        aa_over_current_enable(true);
    }

    if(run_count > STEADY_STATE_THRESHOLD)
    {
        run_count = STEADY_STATE_THRESHOLD + 1;
        if (anode.output_voltage < ANODE_UNDER_VOLTAGE_COUNTS) 
        {
            anode.common.error_adc = anode.output_voltage;
            anode.common.error_code = ANODE_UNDER_VOLTAGE_ERROR;
        }
        else if (((anode.x_voltage > (anode.y_voltage * 1.25)) || (anode.x_voltage < (anode.y_voltage * 0.75))) && anode.mode == QUADRATIC_BOOST)
        {
            voltage_imbalance_counter++;
            if(voltage_imbalance_counter >= VOLTAGE_IMBALANCE_THRESHOLD) {
                if(anode.x_voltage > anode.y_voltage) {
                    anode.common.error_adc = anode.x_voltage - anode.y_voltage;
                } else {
                    anode.common.error_adc = anode.y_voltage - anode.x_voltage;
                }
                anode.common.error_code = ANODE_VOLTAGE_IMBALANCE_ERROR;
            }
        } else voltage_imbalance_counter = 0;
    }
            
    if(anode.common.error_code != ANODE_NO_ERROR)
    {
        anode.common.current_state = anode_error_handler();
    }
}

static void error_clear_check(void)
{
    // check if the readings have returned to nominal
    if((anode.output_current < ANODE_OVER_CURRENT_COUNTS) &&
       (anode.x_voltage < 0.9 * (ANODE_OVER_VOLTAGE_COUNTS / 2)) &&
       (anode.y_voltage < 0.9 * (ANODE_OVER_VOLTAGE_COUNTS / 2)))
    {
        anode.common.current_state = anode_error_cleared_handler();
    }
}

void anode_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(anode.common.error_code == ANODE_NO_ERROR)
    {
        if((control_saturated == true) && 
                (anode.x_pwm_output < (ap_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS)) &&
                (anode.y_pwm_output < (ap_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }
        
        if(anode.target_voltage > voltage_command)
        {
            voltage_command++;  // Slowly ramp voltage to avoid overshoot
        }
        if(anode.target_voltage < voltage_command)
        {
            voltage_command = anode.target_voltage;  // Step down if target voltage is decreased, no ramp down
        }

        mode_select();

        switch(anode.mode)
        {
            case QUADRATIC_BOOST:
                if(anode.output_voltage > (voltage_command + ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    // if PWM is non-zero, subtract one
                    if (anode.x_pwm_output && anode.y_pwm_output)
                    {
                        anode.x_pwm_output--;
                        anode.y_pwm_output--;
                    }
                }
                if(anode.output_voltage < (voltage_command - ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    anode.x_pwm_output++;
                    anode.y_pwm_output++;
                }
                break;
            case TRANSITION_TO_QUAD_BOOST:
                anode.x_pwm_output++;
                if((anode.x_pwm_output != anode.y_pwm_output) && anode.y_pwm_output)
                {
                    anode.y_pwm_output--;
                }
                break;
            case SINGLE_BOOST:
                if(anode.output_voltage > (voltage_command + ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    // if PWM is non-zero, subtract one
                    if (anode.y_pwm_output)
                    {
                        anode.y_pwm_output--;
                    }
                }
                if(anode.output_voltage < (voltage_command - ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    anode.y_pwm_output++;
                }
                break;
            case TRANSITION_TO_SINGLE_BOOST:
                // in theory, PWM should always be non-zero in this mode, but better safe than sorry
                if(anode.x_pwm_output)
                {
                    anode.x_pwm_output--;
                }
                break;
        }

        if(anode.x_pwm_output >= ap_get_max_pwm())
        {
            anode.x_pwm_output = ap_get_max_pwm() - 1;
            if(control_saturated == false)
            {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_ANODE);
                control_saturated = true;
            }
        }
        if(anode.y_pwm_output >= ap_get_max_pwm())
        {
            anode.y_pwm_output = ap_get_max_pwm() - 1;
            if(control_saturated == false)
            {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_ANODE);
                control_saturated = true;
            }
        }

        ap_set_pwm(anode.x_pwm_output, X_PWM);
        ap_set_pwm(anode.y_pwm_output, Y_PWM);
    }
}

void anode_on_state(void)
{
    power_check_on_state();
    
    // only perform control if there was no error
    if(anode.common.error_code == ANODE_NO_ERROR)
    {
        if((control_saturated == true) && 
           (anode.x_pwm_output < (ap_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS)) &&
           (anode.y_pwm_output < (ap_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }

        mode_select();

        switch(anode.mode)
        {
            case QUADRATIC_BOOST:
                if(anode.output_voltage > (anode.target_voltage + ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    // if PWM is non-zero, subtract one
                    if (anode.x_pwm_output && anode.y_pwm_output)
                    {
                        anode.x_pwm_output--;
                        anode.y_pwm_output--;
                    }
                }
                if(anode.output_voltage < (anode.target_voltage - ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    anode.x_pwm_output++;
                    anode.y_pwm_output++;
                }
                break;
            case TRANSITION_TO_QUAD_BOOST:
                anode.x_pwm_output++;
                if((anode.x_pwm_output != anode.y_pwm_output) && anode.y_pwm_output)
                {
                    anode.y_pwm_output--;
                }
                break;
            case SINGLE_BOOST:
                if(anode.output_voltage > (anode.target_voltage + ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    // if PWM is non-zero, subtract one
                    if (anode.y_pwm_output)
                    {
                        anode.y_pwm_output--;
                    }
                }
                if(anode.output_voltage < (anode.target_voltage - ANODE_POWER_GOOD_RANGE_COUNTS))
                {
                    anode.y_pwm_output++;
                }
                break;
            case TRANSITION_TO_SINGLE_BOOST:
                // in theory, PWM should never be non-zero in this mode, but better safe than sorry
                if(anode.x_pwm_output)
                {
                    anode.x_pwm_output--;
                }
                break;
        }

        if(anode.x_pwm_output >= ap_get_max_pwm())
        {
            anode.x_pwm_output = ap_get_max_pwm() - 1;
            if(control_saturated == false)
            {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_ANODE);
                control_saturated = true;
            }
        }
        if(anode.y_pwm_output >= ap_get_max_pwm())
        {
            anode.y_pwm_output = ap_get_max_pwm() - 1;
            if(control_saturated == false)
            {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_ANODE);
                control_saturated = true;
            }
        }

        ap_set_pwm(anode.x_pwm_output, X_PWM);
        ap_set_pwm(anode.y_pwm_output, Y_PWM);
    }
}

void anode_error_state(void)
{
    error_clear_check();
}
