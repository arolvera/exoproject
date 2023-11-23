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
#include "magnet_event_handlers.h"
#include "magnet_mcu.h"
#include "magnet_pwm.h"
#include "magnet_state_handlers.h"

#define POWER_GOOD_TIMEOUT_THRESHOLD     5000 // Trips through the control loop
#define CONTROL_SATURATION_HYSTERESIS      10 // Counts

static uint16_t run_count;
static bool control_saturated;

void msh_reset_counters()
{
    run_count = 0;
    control_saturated = false;
}

static void power_check_startup_state(void) 
{
    run_count++;
    if(magnet.output_current > MAGNET_MAX_I_OUT_COUNTS)
    { 
        magnet.common.error_adc = magnet.output_current;
        magnet.common.error_code = MAGNET_OVER_CURRENT_ERROR;
    }
    else if(magnet.output_voltage > MAGNET_MAX_V_OUT_COUNTS)
    {
        magnet.common.error_adc = magnet.output_voltage;
        magnet.common.error_code = MAGNET_OVER_VOLTAGE_ERROR;
    }
    else if(run_count > POWER_GOOD_TIMEOUT_THRESHOLD)
    {
        magnet.common.error_code = MAGNET_POWER_GOOD_TIMEOUT_ERROR;
    }
    
    if(magnet.common.error_code != MAGNET_NO_ERROR)
    {
        magnet.common.current_state = magnet_error_handler();
    }
}

static void power_check_on_state(void) 
{
    if(magnet.output_current > MAGNET_MAX_I_OUT_COUNTS)
    {
        magnet.common.error_adc = magnet.output_current;
        magnet.common.error_code = MAGNET_OVER_CURRENT_ERROR;
    }
    else if(magnet.output_voltage > MAGNET_MAX_V_OUT_COUNTS)
    {
        magnet.common.error_adc = magnet.output_voltage;
        magnet.common.error_code = MAGNET_OVER_VOLTAGE_ERROR;
    }
    
    if(magnet.common.error_code != MAGNET_NO_ERROR)
    {
        magnet.common.current_state = magnet_error_handler();
    }
}

static void error_clear_check(void)
{
    // check if the readings have returned to nominal
    if((magnet.output_current < MAGNET_MAX_I_OUT_COUNTS) &&
            (magnet.output_voltage < MAGNET_MAX_V_OUT_COUNTS))
    {
        magnet.common.current_state = magnet_error_cleared_handler();
    }    
}

void magnet_startup_state(void)
{
    power_check_startup_state();
    // only perform control if there was no error
    if(magnet.common.error_code == MAGNET_NO_ERROR)
    {
        if(magnet.output_current < magnet.target_current)
        {
            if(magnet.pwm_output < mp_get_max_pwm())
            {
                magnet.pwm_output++;
                mp_set_pwm(magnet.pwm_output);
            } 
            else if(control_saturated == false)
            {
                cc_error_report(CONTROL_SATURATED, 0, COMM_ID_MAGNET);
                control_saturated = true;
            }
        }

        if(magnet.output_current >= magnet.target_current)
        {
            magnet.common.current_state = magnet_power_good_handler();
        }
    }
}

void magnet_on_state(void)
{
    power_check_on_state();
    // only perform control if there was no error and the targets are nonzero
    if(magnet.common.error_code == MAGNET_NO_ERROR)
    {
        // reset the control saturation trackers if control is no longer saturated
        if((control_saturated == true) &&
                (magnet.pwm_output < (mp_get_max_pwm() - CONTROL_SATURATION_HYSTERESIS)))
        {
            control_saturated = false;
        }
        
        if(magnet.target_current > MAGNET_POWER_GOOD_RANGE_COUNTS)
        {
            // highly sophisticated plus one minus one control
            if(magnet.output_current < (magnet.target_current - MAGNET_POWER_GOOD_RANGE_COUNTS))
            {
                if(magnet.pwm_output < mp_get_max_pwm())
                {
                    magnet.pwm_output++;
                    mp_set_pwm(magnet.pwm_output);
                } 
                else if(control_saturated == false)
                {
                    cc_error_report(CONTROL_SATURATED, 0, COMM_ID_MAGNET);
                    control_saturated = true;
                }
            }
            else if(magnet.output_current > (magnet.target_current + MAGNET_POWER_GOOD_RANGE_COUNTS))
            {
                if(magnet.pwm_output) // don't let it go below zero
                {
                    magnet.pwm_output--;
                    mp_set_pwm(magnet.pwm_output);
                }
            }
        }
    }
}

void magnet_error_state(void)
{
    error_clear_check();
}
