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

#include "dac/hal_dac.h"
#include "valve_event_handlers.h"
#include "valve_mcu.h"
#include "valve_pwm.h"
#include "valve_state_handlers.h"

#define CONTROL_MAX    0x00007FFF
#define PID_BIT_SHIFT  3

#define CAT_P_GAIN     0x04E2 // 0x001E - Fritz
#define CAT_I_GAIN     0x0018 // 0x001E - Fritz
#define CAT_D_GAIN     0x0000 // 0x000E - Fritz

#define ANODE_P_GAIN   0x02BC // 0x001E - Fritz
#define ANODE_I_GAIN   0x0004 // 0x0010 - Fritz
#define ANODE_D_GAIN   0x0000 // 0x0006 - Fritz

static int16_t cat_last_pressure;
static int16_t cat_last_error;
static int32_t cat_control_value;
static int16_t cat_integral;

static int16_t anode_last_pressure;
static int16_t anode_last_error;
static int32_t anode_control_value;
static int16_t anode_integral;

void vsh_reset_counters(void)
{
    cat_last_pressure = 0;
    cat_last_error = 0;
    cat_control_value = 0;
    cat_integral = 0;

    anode_last_pressure = 0;
    anode_last_error = 0;
    anode_control_value = 0;
    anode_integral = 0;
}

static void set_cat_high_flow(void)
{
    vp_set_pwm(valve.cat_high_flow_setpoint);
}

static void set_cat_low_flow(void)
{
    int16_t error;
    int16_t derivative;

    error = (int16_t)valve.cat_low_flow_setpoint - valve.cathode_pressure;
    cat_integral = cat_integral + error*0.001;
    derivative = (error - cat_last_error)/0.001; //valve.cathode_pressure - cat_last_pressure;
    //cat_last_pressure = valve.cathode_pressure;

    cat_control_value = //cat_control_value +
        (((int32_t)error) * CAT_P_GAIN) + // -
        (((int32_t)cat_integral) * CAT_I_GAIN) + // -
        (((int32_t)derivative) * CAT_D_GAIN);

    cat_last_error = error;

    if (cat_control_value > CONTROL_MAX)
    {
        cat_control_value = CONTROL_MAX;
    }
    else if(cat_control_value < 0)
    {
        cat_control_value = 0;
    }

    valve.cat_low_flow_dac = (uint16_t)(cat_control_value >> PID_BIT_SHIFT);
    dac_set(valve.cat_low_flow_dac, CATHODE_LF_DAC);
}

static void set_anode_flow(void)
{
    int16_t error;
    int16_t derivative;

    error = (int16_t)valve.anode_flow_setpoint - valve.anode_pressure;
    anode_integral = anode_integral + error*0.001;
    derivative = (error - anode_last_error)/0.001;
    //anode_last_pressure = valve.anode_pressure;

    anode_control_value = //anode_control_value +
        (((int32_t)error) * ANODE_P_GAIN) +
        (((int32_t)anode_integral) * ANODE_I_GAIN) +
        (((int32_t)derivative) * ANODE_D_GAIN);
    anode_last_error = error;

//    error = (int16_t)valve.anode_flow_setpoint - valve.anode_pressure;
//    derivative = valve.anode_pressure - anode_last_pressure;
//    anode_last_pressure = valve.anode_pressure;
//
//    anode_control_value = anode_control_value +
//        (((int32_t)error) * ANODE_P_GAIN) -
//        (((int32_t)anode_last_error) * ANODE_I_GAIN) -
//        (((int32_t)derivative) * ANODE_D_GAIN);
//    anode_last_error = error;

    if (anode_control_value > CONTROL_MAX)
    {
        anode_control_value = CONTROL_MAX;
    }
    else if(anode_control_value < 0)
    {
        anode_control_value = 0;
    }

    valve.anode_dac = (uint16_t)(anode_control_value >> PID_BIT_SHIFT);
    dac_set(valve.anode_dac, ANODE_FLOW_DAC);
}

// no errors for the valves yet, but leaving this for the future
static void error_check(void) 
{            
    if(valve.common.error_code != VALVE_NO_ERROR)
    {
        valve.common.current_state = valve_error_handler();
    }
}

static void error_clear_check(void)
{    
    // check if the readings have returned to nominal
    if(valve.common.error_code == VALVE_NO_ERROR)
    {
        valve.common.current_state = valve_error_cleared_handler();
    }
}

void valve_on_state(void)
{
    error_check();
    
    // only perform control if there was no error
    if(valve.common.error_code == VALVE_NO_ERROR)
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