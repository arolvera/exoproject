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
#include "keeper_hardware_port.h"
#include "keeper_mcu.h"
#include "keeper_pwm.h"
#include "keeper_state_handlers.h"

#define PWM_START_POINT                 25
#define STANDARD_UNDER_CURRENT_TIMEOUT  5     // milliseconds
#define RESTART_UNDER_CURRENT_TIMEOUT   1000  // milliseconds

states_t keeper_on_command_handler(void)
{
    ka_over_current_enable(true);
    ksh_set_counters(STANDARD_UNDER_CURRENT_TIMEOUT);
    keeper_starter_enable(false);
    khp_spark_detect_enable(false);  //RE enable
    cc_state_change_report(STARTUP_STATE, COMMANDED_ON, COMM_ID_KEEPER);
    return STARTUP_STATE;
}

states_t keeper_spark_detected_handler(void)
{
    keeper_starter_enable(false);
    keeper_spark_detected();
    return STARTUP_STATE;
}

states_t keeper_stable_handler(void)
{
    khp_spark_detect_enable(false);
    ka_over_current_enable(false);
    keeper_starter_enable(false);
    keeper.pwm_output = PWM_START_POINT;
    kp_set_flyback_pwm(keeper.pwm_output);
    ksh_set_counters(STANDARD_UNDER_CURRENT_TIMEOUT);
    cc_state_change_report(ON_STATE, SPARK_DETECTED, COMM_ID_KEEPER);
    return ON_STATE;
}

states_t keeper_restart_handler(void)
{
    ka_over_current_enable(true);
    ksh_set_counters(RESTART_UNDER_CURRENT_TIMEOUT);
    cc_state_change_report(ON_STATE, COMMANDED_ON, COMM_ID_KEEPER);
    return ON_STATE;
}

states_t keeper_off_command_handler(void)
{
    keeper_starter_enable(false);
    keeper.pwm_output = 0;
    kp_set_flyback_pwm(keeper.pwm_output);
    khp_spark_detect_enable(false);
    cc_state_change_report(OFF_STATE, COMMANDED_OFF, COMM_ID_KEEPER);
    return OFF_STATE;
}

states_t keeper_error_handler(void)
{
    keeper_starter_enable(false);
    keeper.pwm_output = 0;
    kp_set_flyback_pwm(keeper.pwm_output);
    khp_spark_detect_enable(false);
    cc_error_report(keeper.common.error_code, keeper.common.error_adc, COMM_ID_KEEPER);
    cc_state_change_report(ERROR_STATE, ERROR_DETECTED, COMM_ID_KEEPER);
    return ERROR_STATE;
}

states_t keeper_error_cleared_handler(void)
{
    keeper.common.error_code = KEEPER_NO_ERROR;
    keeper.common.error_adc = 0;
    cc_error_report(keeper.common.error_code, keeper.common.error_adc, COMM_ID_KEEPER);
    cc_state_change_report(OFF_STATE, ERROR_CLEARED, COMM_ID_KEEPER);
    return OFF_STATE;
}
