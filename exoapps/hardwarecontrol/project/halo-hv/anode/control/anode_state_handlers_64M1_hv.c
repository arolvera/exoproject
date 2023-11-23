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

#include "anode_control.h"
#include "anode_event_handlers.h"
#include "anode_pwm.h"
#include "anode_state_handlers.h"
#include "common_DAC.h"
#include "component_communication.h"
#include <stddef.h>

#define SPARK_TIMEOUT_THRESHOLD       10000 // Trips through the control loop
#define STEADY_STATE_THRESHOLD        50    // Trips through the control loop

#define CURRENT_LOW_THRESHOLD         40  // Counts
#define LOW_CURRENT_TIMEOUT           50  // Control cycles before low current error
#define CONTROL_SATURATION_HYSTERESIS 10  // Counts

#define START_WITHOUT_SPARK_THRESHOLD   0.5 // Threshold to say the anode started 
                                            // even if no spark interrupt was triggered
                                            // in A

static uint8_t low_current_counter = 0;
static uint16_t voltage_command = 0;
static uint16_t run_count = 0;
static bool control_saturated = false;

void ash_reset_counters()
{
    low_current_counter = 0;
    voltage_command = 0;
    run_count = 0;
    control_saturated = false;
}

static void power_check_startup_state(void) 
{
    run_count++;
    
    if (anode.output_voltage > ANODE_OVER_VOLTAGE_COUNTS)
    { 
        common.error_ADC = anode.output_voltage;
        common.error_code = ANODE_OVER_VOLTAGE_ERROR;
    }
    else if (anode.output_current > ANODE_OVER_CURRENT_COUNTS) 
    {
        common.error_ADC = anode.output_current;
        common.error_code = ANODE_OVER_CURRENT_ERROR;
    }
    else if (run_count > SPARK_TIMEOUT_THRESHOLD)
    {
        common.error_code = ANODE_SPARK_TIMEOUT_ERROR;
    }
    
    if(common.error_code != ANODE_NO_ERROR)
    {
        common.current_state = anode_error_handler();
    } 
    // Start without spark check
    else if((anode.output_current > (START_WITHOUT_SPARK_THRESHOLD * ANODE_COUNTS_PER_AMPERE)) &&
            (common.current_state == STARTUP_STATE))
    {
        common.current_state = anode_spark_detected_handler();
    }
}

static void power_check_on_state(void) 
{
    if (anode.output_voltage > ANODE_OVER_VOLTAGE_COUNTS)
    { 
        common.error_ADC = anode.output_voltage;
        common.error_code = ANODE_OVER_VOLTAGE_ERROR;
    }
    // @fixme The over-current limit should be lowered (to around 2.3A) and this 
    // check should be moved to the post plasma burning checks (with over 
    // voltage). I'm going to leave it how we tested it until there is an 
    // opportunity for more testing.
    else if (anode.output_current > ANODE_OVER_CURRENT_COUNTS) 
    {
        common.error_ADC = anode.output_current;
        common.error_code = ANODE_OVER_CURRENT_ERROR;
    }
    else if (anode.output_current < CURRENT_LOW_THRESHOLD) 
    {
        low_current_counter++;
        if(low_current_counter > LOW_CURRENT_TIMEOUT)
        {
            common.error_ADC = anode.output_current;
            common.error_code = ANODE_UNDER_CURRENT_ERROR;
        }    
    } else low_current_counter = 0;
    
    run_count++;
    if(run_count > STEADY_STATE_THRESHOLD)
    {
        run_count = STEADY_STATE_THRESHOLD + 1;
        if (anode.output_voltage < ANODE_UNDER_VOLTAGE_COUNTS) 
        {
            common.error_ADC = anode.output_voltage;
            common.error_code = ANODE_UNDER_VOLTAGE_ERROR;
        }
    }
            
    if(common.error_code != ANODE_NO_ERROR)
    {
        common.current_state = anode_error_handler();
    }
}

static void error_clear_check(void)
{    
    // check if the readings have returned to nominal
    if((anode.output_current < ANODE_OVER_CURRENT_COUNTS) &&
            (anode.output_voltage < ANODE_OVER_VOLTAGE_COUNTS))
    {
        common.current_state = anode_error_cleared_handler();
    }
}

void anode_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(common.error_code == ANODE_NO_ERROR)
    {
        if((control_saturated == true) && 
                (anode.PWM_output < (ANODE_MAX_PWM - CONTROL_SATURATION_HYSTERESIS)))
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
        if(anode.output_voltage > voltage_command)
        {
            // if PWM is non-zero, subtract one
            if (anode.PWM_output)
            {
                anode.PWM_output--;
            }
        }
        if(anode.output_voltage < voltage_command)
        {
            anode.PWM_output++;
        }
        
        if(anode.PWM_output > ANODE_MAX_PWM)
        {
            anode.PWM_output = ANODE_MAX_PWM;
            if(control_saturated == false)
            {
                cc_control_saturated();
                control_saturated = true;
            }
        }
        ap_set_PWM(anode.PWM_output);
    }
}

void anode_on_state(void)
{
    power_check_on_state();
    
    // only perform control if there was no error
    if(common.error_code == ANODE_NO_ERROR)
    {
        if((control_saturated == true) && 
                (anode.PWM_output < (ANODE_MAX_PWM - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }
        
        if(anode.output_voltage > anode.target_voltage)
        {
            // if PWM is non-zero, subtract one
            if (anode.PWM_output)
            {
                anode.PWM_output--;
            }
        }
        if(anode.output_voltage < anode.target_voltage)
        {
            anode.PWM_output++;
        }
        
        if(anode.PWM_output > ANODE_MAX_PWM)
        {
            anode.PWM_output = ANODE_MAX_PWM;
            if(control_saturated == false)
            {
                cc_control_saturated();
                control_saturated = true;
            }
        }
        ap_set_PWM(anode.PWM_output);       
    }
}

void anode_error_state(void)
{
    error_clear_check();
}