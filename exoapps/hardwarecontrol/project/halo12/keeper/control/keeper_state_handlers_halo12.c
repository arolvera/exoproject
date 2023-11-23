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

#include "component_communication.h"
#include "keeper_adc.h"
#include "keeper_control.h"
#include "keeper_event_handlers.h"
#include "keeper_mcu.h"
#include "keeper_pwm.h"
#include "keeper_state_handlers.h"

#define SPARK_TIMEOUT_THRESHOLD 20000   // Trips through the control loop
#define STEADY_STATE_THRESHOLD 2000       // Trips through the control loop
#define CURRENT_LOW_THRESHOLD 100       // Counts
#define LOW_CURRENT_TIMEOUT 5           // Control cycles before low current error
#define CONTROL_SATURATION_HYSTERESIS 10// Counts
#define MAX_PWM_STARTER_COUNTS 100
#define STARTUP_STABLE_THRESHOLD 5000// Time in ms required to consider the start stable

//PI Control Loop Values
#define VOLTAGE_CONTROL_I_SATURATION 60000
#define VOLTAGE_CONTROL_I_SCALAR 1000
#define VOLTAGE_CONTROL_P_SCALAR 75
#define VOLTAGE_CONTROL_I_INPUT_SCALAR 5

//PI Control Loop Values
#define CURRENT_CONTROL_I_SATURATION 60000
#define CURRENT_CONTROL_I_SCALAR 1000
#define CURRENT_CONTROL_P_SCALAR 75
#define CURRENT_CONTROL_I_INPUT_SCALAR 5

#define START_WITHOUT_SPARK_THRESHOLD 0.25  // Threshold to say the keeper started      \
                                            // even if no spark interrupt was triggered \
                                            // in A
#define START_RESTART_STARTUP_THRESHOLD 0.10// Threshold to switch back to voltage control \
                                            // mode and spark the keeper


static uint8_t low_current_counter = 0;
static uint16_t target_voltage = 0;
static int32_t voltage_control_state = 0;
static int32_t current_control_state = 0;
static uint16_t run_count = 0;
static uint16_t startup_stable_count = 0;
static bool control_saturated = false;
static bool startup_control_cv = true;
static bool sparkDetected = false;
static bool keeper_starter_on = false;
static bool keeper_starter_at_voltage = false;

void ksh_set_counters(uint16_t lct)
{
    low_current_counter = 0;
    run_count = 0;
    voltage_control_state = 0;
    current_control_state = 0;
    target_voltage = 0;
    startup_stable_count = 0;
    control_saturated = false;
    startup_control_cv = true;
    sparkDetected = false;
    keeper_starter_at_voltage = false;
}

void keeper_spark_detected(void)
{
    sparkDetected = true;
}

void keeper_starter_enable(bool enabled){
    kp_starter_enable(enabled);
    keeper_starter_on = enabled;
}

static void keeper_starter_handler(void){
    bool keeper_starter_primed = keeper.flyback_voltage > (keeper.target_voltage * KEEPER_STARTER_STARTING_RATIO);
    bool keeper_starter_above_target = keeper.starter_voltage > (KEEPER_STARTER_TARGET_COUNTS);
    bool keeper_starter_below_target = keeper.starter_voltage < (KEEPER_STARTER_TARGET_COUNTS * KEEPER_STARTER_STARTING_RATIO);

    if(!keeper_starter_on && !keeper_starter_at_voltage && keeper_starter_primed){
        keeper_starter_enable(true);
    }
    if(keeper_starter_on && !keeper_starter_primed) {
        keeper_starter_enable(false);
    }

    if(keeper_starter_on && keeper_starter_above_target){
        keeper_starter_enable(false);
        keeper_starter_at_voltage = true;
    }
    if(!keeper_starter_on && keeper_starter_at_voltage && keeper_starter_below_target) {
        keeper_starter_enable(true);
    }
}

static void power_check_startup_state(void)
{
    run_count++;

    if(run_count > SPARK_TIMEOUT_THRESHOLD) {
        //keeper.common.error_code = KEEPER_SPARK_TIMEOUT_ERROR;
    }

    if(keeper.common.error_code != KEEPER_NO_ERROR) {
        keeper.common.current_state = keeper_error_handler();
    }
    // Start without spark check
    //else if((keeper.output_current > (START_WITHOUT_SPARK_THRESHOLD * KEEPER_COUNTS_PER_AMPERE)) &&
    //        (keeper.common.current_state == STARTUP_STATE))
    //{
    if((startup_stable_count > STARTUP_STABLE_THRESHOLD) && (keeper.common.current_state == STARTUP_STATE)) {
        //keeper.common.current_state = keeper_stable_handler();
    }
}

static void power_check_on_state(void)
{

    if(keeper.output_current < CURRENT_LOW_THRESHOLD) {
        low_current_counter++;
        if(low_current_counter > LOW_CURRENT_TIMEOUT) {
            keeper.common.error_adc = keeper.output_current;
            keeper.common.error_code = KEEPER_UNDER_CURRENT_ERROR;
        }
    } else
        low_current_counter = 0;

    run_count++;

    if(run_count == STEADY_STATE_THRESHOLD) {
        ka_over_current_enable(true);
    }

    if(run_count > STEADY_STATE_THRESHOLD) {
        run_count = STEADY_STATE_THRESHOLD + 1;
        if(keeper.flyback_voltage > KEEPER_OVER_VOLTAGE_COUNTS_ON) {
            keeper.common.error_adc = keeper.flyback_voltage;
            keeper.common.error_code = KEEPER_OVER_VOLTAGE_ON_STATE_ERROR;
        } else if(keeper.flyback_voltage * keeper.output_current > KEEPER_MAX_POWER_COUNTS_SQR) {
            keeper.common.error_adc = keeper.flyback_voltage * keeper.output_current;
            keeper.common.error_code = KEEPER_OVER_POWER_ERROR;
        }
    }

    if(keeper.common.error_code != KEEPER_NO_ERROR) {
        keeper.common.current_state = keeper_error_handler();
    }
}

static void error_clear_check(void)
{
    // check if the readings have returned to nominal
    if((keeper.output_current < KEEPER_MAX_I_OUT_COUNTS) && (keeper.flyback_voltage < KEEPER_MAX_V_OUT_COUNTS) && (keeper.starter_voltage < KEEPER_OVER_VOLTAGE_COUNTS_STARTER)) {
        keeper.common.current_state = keeper_error_cleared_handler();
    }
}

void keeper_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(keeper.common.error_code == KEEPER_NO_ERROR) {
        // reset the control saturation tracker if control is no longer saturated
        if((control_saturated == true) && (keeper.pwm_output < (kp_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS))) {
            control_saturated = false;
        }

        keeper.output_current_filtered = (2*keeper.output_current_filtered + keeper.output_current)/3;
        keeper.starter_voltage_filtered = (2*keeper.starter_voltage_filtered + keeper.starter_voltage)/3;

        if (target_voltage < keeper.target_voltage){
            target_voltage++;
        }
        if (target_voltage > keeper.target_voltage){
            target_voltage = keeper.target_voltage;
        }

        int16_t control_output = 0;

        if(startup_control_cv == true) {
            //keeper_starter_enable(true);
            keeper_starter_handler();
            if(sparkDetected || (keeper.output_current_filtered > (START_WITHOUT_SPARK_THRESHOLD * KEEPER_COUNTS_PER_AMPERE))) {
                startup_control_cv = false;
                sparkDetected = false;
                keeper_starter_enable(false);
                keeper_starter_at_voltage = false;
                startup_stable_count = 0;
            }
        }

        if(startup_control_cv == false) {
            if(keeper.output_current_filtered < (START_RESTART_STARTUP_THRESHOLD * KEEPER_COUNTS_PER_AMPERE)) {
                startup_control_cv = true;
            }
        }

        //Voltage Control Loop
        if(startup_control_cv == true) {
            int16_t voltage_error = target_voltage - keeper.flyback_voltage;
            int16_t voltage_control_output = 0;

            voltage_control_state = voltage_control_state + (voltage_error / VOLTAGE_CONTROL_I_INPUT_SCALAR);

            if(voltage_control_state > VOLTAGE_CONTROL_I_SATURATION) {
                voltage_control_state = VOLTAGE_CONTROL_I_SATURATION;
            }
            if(voltage_control_state < 0) {
                voltage_control_state = 0;
            }

            voltage_control_output = (voltage_control_state / VOLTAGE_CONTROL_I_SCALAR) + (voltage_error / VOLTAGE_CONTROL_P_SCALAR);

            control_output = voltage_control_output;
        }

        //Current Control Loop
        if(startup_control_cv == false) {

            int16_t current_error = keeper.target_current - keeper.output_current;
            int16_t current_control_output = 0;

            current_control_state = current_control_state + current_error / CURRENT_CONTROL_I_INPUT_SCALAR;

            if(current_control_state > CURRENT_CONTROL_I_SATURATION) {
                current_control_state = CURRENT_CONTROL_I_SATURATION;
            }
            if(current_control_state < 10000) {
                current_control_state = 10000;
            }

            current_control_output = current_control_state / CURRENT_CONTROL_I_SCALAR + current_error / CURRENT_CONTROL_P_SCALAR;
            control_output = current_control_output;
        }

        if(control_output < 0) {
            control_output = 0;
        }
        if(control_output > 80) {
            control_output = 80;
        }

        if(control_output > MAX_PWM_STARTER_COUNTS) {
            control_output = MAX_PWM_STARTER_COUNTS;
            if(control_saturated == false) {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_KEEPER);
                control_saturated = true;
            }
        }

        keeper.pwm_output = (uint16_t)control_output;

        startup_stable_count++;

        kp_set_flyback_pwm(keeper.pwm_output);
    }
}

void keeper_on_state(void)
{
    power_check_on_state();

    // only perform control if there was no error
    if(keeper.common.error_code == KEEPER_NO_ERROR) {
        // reset the control saturation tracker if control is no longer saturated
        if((control_saturated == true) && (keeper.pwm_output < (kp_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS))) {
            control_saturated = false;
        }

        if(keeper.output_current > (keeper.target_current + KEEPER_POWER_GOOD_RANGE_COUNTS)) {
            if(keeper.pwm_output) {
                keeper.pwm_output--;
            }
        }
        if(keeper.output_current < (keeper.target_current - KEEPER_POWER_GOOD_RANGE_COUNTS)) {
            if(keeper.pwm_output < kp_get_max_pwm()) {
                keeper.pwm_output++;
            } else {
                keeper.pwm_output = kp_get_max_pwm();
                if(control_saturated == false) {
                    cc_error_report(CONTROL_SATURATED, 0, COMM_ID_KEEPER);
                    control_saturated = true;
                }
            }
        }
        kp_set_flyback_pwm(keeper.pwm_output);
    }
}

void keeper_error_state(void)
{
    error_clear_check();
}