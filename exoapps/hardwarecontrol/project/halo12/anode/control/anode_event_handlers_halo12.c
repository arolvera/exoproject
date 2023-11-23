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
#include "anode_hardware_port.h"
#include "anode_mcu.h"
#include "anode_pwm.h"
#include "anode_state_handlers.h"
#include "component_communication.h"

#define PWM_START_POINT            65
#define QUADRATIC_THRESHOLD        250 // Volts

states_t anode_on_command_handler(void)
{
    aa_over_current_enable(true);
    if(anode.target_voltage > (QUADRATIC_THRESHOLD * ANODE_COUNTS_PER_VOLT_OUTPUT))
    {
        anode.mode = QUADRATIC_BOOST;
    } else {
        anode.mode = SINGLE_BOOST;
    }
    ash_reset_counters();
    ahp_input_power_enable(true);
    ahp_spark_detect_enable(true);
    cc_state_change_report(STARTUP_STATE, COMMANDED_ON, COMM_ID_ANODE);
    return STARTUP_STATE;
}

states_t anode_spark_detected_handler(void)
{
    ahp_spark_detect_enable(false);
    aa_over_current_enable(false);
    anode.y_pwm_output = PWM_START_POINT;
    ap_set_pwm(anode.y_pwm_output, Y_PWM);
    if(anode.mode == QUADRATIC_BOOST || anode.mode == TRANSITION_TO_QUAD_BOOST){
        anode.x_pwm_output = PWM_START_POINT;
        ap_set_pwm(anode.x_pwm_output, X_PWM);
        anode.mode = QUADRATIC_BOOST;
    } else {
        anode.x_pwm_output = 0;
        ap_set_pwm(anode.x_pwm_output, X_PWM);
        anode.mode = SINGLE_BOOST;
    }
    ash_reset_counters();
    cc_state_change_report(ON_STATE, SPARK_DETECTED, COMM_ID_ANODE);
    return ON_STATE;
}

states_t anode_off_command_handler(void)
{
    ahp_input_power_enable(false);
    anode.x_pwm_output = 0;
    anode.y_pwm_output = 0;
    ap_set_pwm(anode.x_pwm_output, X_PWM);
    ap_set_pwm(anode.y_pwm_output, Y_PWM);
    ahp_spark_detect_enable(false);
    cc_state_change_report(OFF_STATE, COMMANDED_OFF, COMM_ID_ANODE);
    return OFF_STATE;
}

states_t anode_error_handler(void)
{
    ahp_input_power_enable(false);
    anode.x_pwm_output = 0;
    anode.y_pwm_output = 0;
    ap_set_pwm(anode.x_pwm_output, X_PWM);
    ap_set_pwm(anode.y_pwm_output, Y_PWM);
    ahp_spark_detect_enable(false);
    cc_error_report(anode.common.error_code, anode.common.error_adc, COMM_ID_ANODE);
    cc_state_change_report(ERROR_STATE, ERROR_DETECTED, COMM_ID_ANODE);
    return ERROR_STATE;
}

states_t anode_error_cleared_handler(void)
{
    anode.common.error_code = ANODE_NO_ERROR;
    anode.common.error_adc = 0;
    cc_error_report(anode.common.error_code, anode.common.error_adc, COMM_ID_ANODE);
    cc_state_change_report(OFF_STATE, ERROR_CLEARED, COMM_ID_ANODE);
    return OFF_STATE;
}