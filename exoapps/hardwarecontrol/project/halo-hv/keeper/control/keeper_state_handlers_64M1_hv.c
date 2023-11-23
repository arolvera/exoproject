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

#include "common_DAC.h"
#include "component_communication.h"
#include "keeper_control.h"
#include "keeper_event_handlers.h"
#include "keeper_state_handlers.h"
#include <stddef.h>

#define VOLTAGE_CONTROL_CLAMP_LOW     50  // Counts
#define VOLTAGE_CONTROL_CLAMP_HIGH    400 // Counts
#define CURRENT_CONTROL_CLAMP_LOW     20  // Counts
#define CURRENT_CONTROL_CLAMP_HIGH    850 // Counts

#define SPARK_TIMEOUT_THRESHOLD       10000 // Trips through the control loop
#define STEADY_STATE_THRESHOLD        50    // Trips through the control loop
#define CURRENT_LOW_THRESHOLD         20  // Counts
#define LOW_CURRENT_TIMEOUT           10  // Control cycles before low current error
#define CONTROL_SATURATION_HYSTERESIS 10  // Counts

#define START_WITHOUT_SPARK_THRESHOLD   0.5 // Threshold to say the keeper started 
                                            // even if no spark interrupt was triggered
                                            // in A

static uint8_t low_current_counter = 0;
static uint16_t voltage_command = 0;
static uint16_t run_count = 0;
static bool control_saturated = false;

void ksh_reset_counters()
{
    low_current_counter = 0;
    voltage_command = 0;
    run_count = 0;
    control_saturated = false;
}

static void power_check_startup_state(void) 
{
    run_count++;
    
    if (keeper.output_voltage > KEEPER_OVER_VOLTAGE_COUNTS_STARTUP)
    { 
        common.error_ADC = keeper.output_voltage;
        common.error_code = KEEPER_OVER_VOLTAGE_STARTUP_STATE_ERROR;
    }
    else if (keeper.output_current > KEEPER_MAX_I_OUT_COUNTS) 
    {
        common.error_ADC = keeper.output_current;
        common.error_code = KEEPER_OVER_CURRENT_ERROR;
    }
    else if (run_count > SPARK_TIMEOUT_THRESHOLD)
    {
        common.error_code = KEEPER_SPARK_TIMEOUT_ERROR;
    }
    
    if(common.error_code != KEEPER_NO_ERROR)
    {
        common.current_state = keeper_error_handler();
    } 
    // Start without spark check
    else if((keeper.output_current > (START_WITHOUT_SPARK_THRESHOLD * KEEPER_COUNTS_PER_AMPERE)) &&
            (common.current_state == STARTUP_STATE))
    {
        common.current_state = keeper_spark_detected_handler();
    }
}

static void power_check_on_state(void) 
{
    if (keeper.output_current < CURRENT_LOW_THRESHOLD) 
    {
        low_current_counter++;
        if(low_current_counter > LOW_CURRENT_TIMEOUT)
        {
            common.error_ADC = keeper.output_current;
            common.error_code = KEEPER_UNDER_CURRENT_ERROR;
        }    
    } else low_current_counter = 0;
    
    run_count++;
    if(run_count > STEADY_STATE_THRESHOLD)
    {
        if (keeper.output_voltage > KEEPER_OVER_VOLTAGE_COUNTS_ON)
        { 
            common.error_ADC = keeper.output_voltage;
            common.error_code = KEEPER_OVER_VOLTAGE_ON_STATE_ERROR;
        }
        else if (keeper.output_current > KEEPER_MAX_I_OUT_COUNTS) 
        {
            common.error_ADC = keeper.output_current;
            common.error_code = KEEPER_OVER_CURRENT_ERROR;
        }
        run_count = STEADY_STATE_THRESHOLD + 1;
    }

    if(common.error_code != KEEPER_NO_ERROR)
    {
        common.current_state = keeper_error_handler();
    }
}

static void error_clear_check(void)
{    
    // check if the readings have returned to nominal
    if((keeper.output_current < KEEPER_MAX_I_OUT_COUNTS) &&
            (keeper.output_voltage < KEEPER_MAX_V_OUT_COUNTS))
    {
        common.current_state = keeper_error_cleared_handler();
    }
}

static uint8_t control_step_size(int16_t error_value)
{
    uint8_t step_size = 1;
    if(error_value < 0)
    {
        error_value *= -1;
    }
    
    if(error_value > 20)
    {
        step_size = 5;
    } 
    else if (error_value > 10)
    {
        step_size = 3;
    }
    else if (error_value > 5)
    {
        step_size = 2;
    }
    return step_size;
}

void keeper_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(common.error_code == KEEPER_NO_ERROR)
    {
        // reset the control saturation tracker if control is no longer saturated
        if((control_saturated == true) && 
                (keeper.DAC_output < (VOLTAGE_CONTROL_CLAMP_HIGH - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }
        
        if(keeper.target_voltage > voltage_command)
        {
            voltage_command++;  // Slowly ramp voltage to avoid overshoot
        }
        if(keeper.target_voltage < voltage_command)
        {
            voltage_command = keeper.target_voltage;  // Step down if target voltage is decreased, no ramp down
        }
        int16_t voltage_error;
        if(keeper.output_voltage > voltage_command)
        {
            if (keeper.DAC_output > VOLTAGE_CONTROL_CLAMP_LOW)
            {
                voltage_error = ((int16_t)keeper.output_voltage - (int16_t)voltage_command);
                keeper.DAC_output -= control_step_size(voltage_error);
            } else {
                keeper.DAC_output = VOLTAGE_CONTROL_CLAMP_LOW;
            }
        }
        if(keeper.output_voltage < voltage_command)
        {
            if (keeper.DAC_output < VOLTAGE_CONTROL_CLAMP_HIGH){
                voltage_error = ((int16_t)voltage_command - (int16_t)keeper.output_voltage);
                keeper.DAC_output += control_step_size(voltage_error);
            } else {
                keeper.DAC_output = VOLTAGE_CONTROL_CLAMP_HIGH;
                if(control_saturated == false)
                {
                    cc_control_saturated();
                    control_saturated = true;
                }
            }
        }
        cd_set_DAC(keeper.DAC_output);
    }
}

void keeper_on_state(void)
{
    power_check_on_state();
    
    // only perform control if there was no error
    if(common.error_code == KEEPER_NO_ERROR)
    {
        // reset the control saturation tracker if control is no longer saturated
        if((control_saturated == true) && 
                (keeper.DAC_output < (CURRENT_CONTROL_CLAMP_HIGH - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }
        
        int16_t current_error;
        if(keeper.output_current > keeper.target_current)
        {
            if (keeper.DAC_output > CURRENT_CONTROL_CLAMP_LOW)
            {
                current_error = ((int16_t)keeper.output_current - (int16_t)keeper.target_current);
                keeper.DAC_output -= control_step_size(current_error);
            } else {
                keeper.DAC_output = CURRENT_CONTROL_CLAMP_LOW;
            }
        }
        if(keeper.output_current < keeper.target_current)
        {
            if(keeper.DAC_output < CURRENT_CONTROL_CLAMP_HIGH)
            {
                current_error = ((int16_t)keeper.target_current - (int16_t)keeper.output_current);
                keeper.DAC_output += control_step_size(current_error);
            } else {
                keeper.DAC_output = CURRENT_CONTROL_CLAMP_HIGH;
                if(control_saturated == false)
                {
                    cc_control_saturated();
                    control_saturated = true;
                }
            }
        }
        cd_set_DAC(keeper.DAC_output);        
    }
}

void keeper_error_state(void)
{
    error_clear_check();
}