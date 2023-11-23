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

#include "valve_control.h"
#include "valve_event_handlers.h"
#include "valve_pwm.h"
#include "valve_state_handlers.h"
#include <stddef.h>

static void set_cat_high_flow(void)
{
    // convert the permil setpoint to a PWM value
    uint16_t PWM = (uint16_t)(((uint32_t)valve.cat_high_flow_setpoint * 
            (uint32_t)VALVE_MAX_PWM) / (uint32_t)VALVE_MAX_HIGH_FLOW);
    vp_set_PWM(PWM, CATHODE_HIGH_FLOW_VALVE);
}

static void set_cat_low_flow(void)
{
    if (valve.cathode_pressure > valve.cat_low_flow_setpoint)
    {
        if(valve.cat_low_flow_PWM)
        {
            valve.cat_low_flow_PWM--;
        }
    }
    else if(valve.cathode_pressure < valve.cat_low_flow_setpoint)
    {
        if(valve.cat_low_flow_PWM < VALVE_MAX_PWM)
        {
            valve.cat_low_flow_PWM++;
        }
    }
    vp_set_PWM(valve.cat_low_flow_PWM, CATHODE_LOW_FLOW_VALVE);
}

static void set_anode_flow(void)
{
    if (valve.anode_pressure > valve.anode_flow_setpoint)
    {
        if(valve.anode_PWM)
        {
            valve.anode_PWM--;
        }
    }
    else if(valve.anode_pressure < valve.anode_flow_setpoint)
    {
        if(valve.anode_PWM < VALVE_MAX_PWM)
        {
            valve.anode_PWM++;
        }
    }
    vp_set_PWM(valve.anode_PWM, ANODE_FLOW_VALVE);
}

// no errors for the valves yet, but leaving this for the future
static void error_check(void) 
{            
    if(common.error_code != VALVE_NO_ERROR)
    {
        common.current_state = valve_error_handler();
    }
}

static void error_clear_check(void)
{    
    // check if the readings have returned to nominal
    if(common.error_code == VALVE_NO_ERROR)
    {
        common.current_state = valve_error_cleared_handler();
    }
}

void valve_on_state(void)
{
    error_check();
    
    // only perform control if there was no error
    if(common.error_code == VALVE_NO_ERROR)
    {
        set_cat_high_flow();
        set_cat_low_flow();
        set_anode_flow();
    }
}

void valve_error_state(void)
{
    error_clear_check();
}