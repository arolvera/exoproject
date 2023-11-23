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
#include "magnet_control.h"
#include "magnet_event_handlers.h"
#include "magnet_mcu.h"
#include "magnet_pwm.h"
#include "magnet_state_handlers.h"
#include <stddef.h>

#define POWER_GOOD_TIMEOUT_THRESHOLD     5000 // Trips through the control loop
#define CONTROL_SATURATION_HYSTERESIS      10 // Counts

static uint16_t run_count;
static bool inner_control_saturated;
static bool outer_control_saturated;

void msh_reset_counters()
{
    run_count = 0;
    inner_control_saturated = false;
    outer_control_saturated = false;
}

static void power_check_startup_state(void) 
{
    run_count++;
    if(magnet.inner_current > MAGNET_MAX_I_OUT_COUNTS)
    { 
        common.error_ADC = magnet.inner_current;
        common.error_code = MAGNET_INNER_CURRENT_ERROR;
    }
    else if(magnet.outer_current > MAGNET_MAX_I_OUT_COUNTS) 
    {
        common.error_ADC = magnet.outer_current;
        common.error_code = MAGNET_OUTER_CURRENT_ERROR;
    }
    else if(magnet.inner_voltage > MAGNET_MAX_V_OUT_COUNTS) 
    {
        common.error_ADC = magnet.inner_voltage;
        common.error_code = MAGNET_INNER_VOLTAGE_ERROR;
    } 
    else if(magnet.outer_voltage > MAGNET_MAX_V_OUT_COUNTS) 
    {
        common.error_ADC = magnet.outer_voltage;
        common.error_code = MAGNET_OUTER_VOLTAGE_ERROR;
    }
    else if(magnet.temperature > MAGNET_MAX_TEMP_COUNTS) 
    {
        common.error_ADC = magnet.temperature;
        common.error_code = MAGNET_TEMPERATURE_ERROR;
    }
    else if(run_count > POWER_GOOD_TIMEOUT_THRESHOLD)
    {
        common.error_code = MAGNET_POWER_GOOD_TIMEOUT_ERROR;
    }
    
    if(common.error_code != MAGNET_NO_ERROR)
    {
        common.current_state = magnet_error_handler();
    }
}

static void power_check_on_state(void) 
{
    if(magnet.inner_current > MAGNET_MAX_I_OUT_COUNTS)
    { 
        common.error_ADC = magnet.inner_current;
        common.error_code = MAGNET_INNER_CURRENT_ERROR;
    }
    else if(magnet.outer_current > MAGNET_MAX_I_OUT_COUNTS) 
    {
        common.error_ADC = magnet.outer_current;
        common.error_code = MAGNET_OUTER_CURRENT_ERROR;
    }
    else if(magnet.inner_voltage > MAGNET_MAX_V_OUT_COUNTS) 
    {
        common.error_ADC = magnet.inner_voltage;
        common.error_code = MAGNET_INNER_VOLTAGE_ERROR;
    } 
    else if(magnet.outer_voltage > MAGNET_MAX_V_OUT_COUNTS) 
    {
        common.error_ADC = magnet.outer_voltage;
        common.error_code = MAGNET_OUTER_VOLTAGE_ERROR;
    }
    else if(magnet.temperature > MAGNET_MAX_TEMP_COUNTS) 
    {
        common.error_ADC = magnet.temperature;
        common.error_code = MAGNET_TEMPERATURE_ERROR;
    }
    
    if(common.error_code != MAGNET_NO_ERROR)
    {
        common.current_state = magnet_error_handler();
    }
}

static void error_clear_check(void)
{
    // check if the readings have returned to nominal
    if((magnet.inner_current < MAGNET_MAX_I_OUT_COUNTS) &&
            (magnet.outer_current < MAGNET_MAX_I_OUT_COUNTS) &&
            (magnet.inner_voltage < MAGNET_MAX_V_OUT_COUNTS) &&
            (magnet.outer_voltage < MAGNET_MAX_V_OUT_COUNTS) &&
            (magnet.temperature < MAGNET_MAX_TEMP_COUNTS))
    {
        common.current_state = magnet_error_cleared_handler();
    }    
}

void magnet_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(common.error_code == MAGNET_NO_ERROR)
    {
        if(magnet.inner_current < magnet.target_inner_current)
        {
            if(magnet.inner_PWM < MAGNET_MAX_PWM) 
            {
                magnet.inner_PWM++;
                mp_set_inner_PWM(magnet.inner_PWM);
            } 
            else if(inner_control_saturated == false)
            {
                cc_control_saturated();
                inner_control_saturated = true;
            }
        }
        
        if(magnet.outer_current < magnet.target_outer_current)
        {
            if(magnet.outer_PWM < MAGNET_MAX_PWM) 
            {
                magnet.outer_PWM++;
                mp_set_outer_PWM(magnet.outer_PWM);
            } 
            else if(outer_control_saturated == false)
            {
                cc_control_saturated();
                outer_control_saturated = true;
            }    
        }
        
        if((magnet.inner_current >= magnet.target_inner_current) &&
                (magnet.outer_current >= magnet.target_outer_current))
        {
            common.current_state = magnet_power_good_handler();
        }
    }
}

void magnet_on_state(void)
{
    power_check_on_state();
    // only perform control if there was no error and the targets are nonzero
    if(common.error_code == MAGNET_NO_ERROR)
    {
        // reset the control saturation trackers if control is no longer saturated
        if((inner_control_saturated == true) && 
                (magnet.inner_PWM < (MAGNET_MAX_PWM - CONTROL_SATURATION_HYSTERESIS)))
        {
            inner_control_saturated = false;
        }
        if((outer_control_saturated == true) && 
                (magnet.outer_PWM < (MAGNET_MAX_PWM - CONTROL_SATURATION_HYSTERESIS)))
        {
            outer_control_saturated = false;
        }
        
        if(magnet.target_inner_current > MAGNET_POWER_GOOD_RANGE_COUNTS) 
        {
            // highly sophisticated plus one minus one control
            if(magnet.inner_current < (magnet.target_inner_current - MAGNET_POWER_GOOD_RANGE_COUNTS)) 
            {
                if(magnet.inner_PWM < MAGNET_MAX_PWM) 
                {
                    magnet.inner_PWM++;
                    mp_set_inner_PWM(magnet.inner_PWM);
                } 
                else if(inner_control_saturated == false)
                {
                    cc_control_saturated();
                    inner_control_saturated = true;
                }
            }
            else if(magnet.inner_current > (magnet.target_inner_current + MAGNET_POWER_GOOD_RANGE_COUNTS)) 
            {
                if(magnet.inner_PWM) // don't let it go below zero
                {
                    magnet.inner_PWM--;
                    mp_set_inner_PWM(magnet.inner_PWM);
                }
            }
        }
        
        if(magnet.target_outer_current > MAGNET_POWER_GOOD_RANGE_COUNTS) 
        {
            if(magnet.outer_current < (magnet.target_outer_current - MAGNET_POWER_GOOD_RANGE_COUNTS)) 
            {
                if(magnet.outer_PWM < MAGNET_MAX_PWM) 
                {
                    magnet.outer_PWM++;
                    mp_set_outer_PWM(magnet.outer_PWM);
                } 
                else if(outer_control_saturated == false)
                {
                    cc_control_saturated();
                    outer_control_saturated = true;
                }
            } 
            else if(magnet.outer_current > (magnet.target_outer_current + MAGNET_POWER_GOOD_RANGE_COUNTS)) 
            {
                if(magnet.outer_PWM) // don't let it go below zero
                {
                    magnet.outer_PWM--;
                    mp_set_outer_PWM(magnet.outer_PWM);
                }
            }
        }
    }
}

void magnet_error_state(void)
{
    error_clear_check();
}
