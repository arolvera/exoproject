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

#include "anode_comp.h"
#include "anode_control.h"
#include "anode_event_handlers.h"
#include "anode_hardware_port.h"
#include "anode_pwm.h"
#include "anode_state_handlers.h"
#include "common_DAC.h"
#include "component_communication.h"
#include <stddef.h>

#define PWM_START_POINT    1200
#define MAX_DAC            0x3FF

states_t anode_on_command_handler(void)
{
    ash_reset_counters();
    ahp_buck_enable();
    ac_spark_detect_enable();
    cd_set_DAC(MAX_DAC);
    // This delay is a bit of a guess and could probably be reduced, but it is 
    // important that the buck is fully turned on before the boost is enabled. 
    for(uint16_t i = 0; i < 1000; i++);
    ahp_boost_enable();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_ON, common.error_ADC);
    return STARTUP_STATE;
}

states_t anode_spark_detected_handler(void)
{
    ac_spark_detect_disable();
    anode.PWM_output = PWM_START_POINT;
    ap_set_PWM(anode.PWM_output);
    ash_reset_counters();
    cc_send_message(BCAST_OUTPUT_CONTROL, SPARK_DETECTED, common.error_ADC);
    return ON_STATE;
}

states_t anode_off_command_handler(void)
{
    ahp_disable();
    anode.PWM_output = 0;
    ap_set_PWM(anode.PWM_output);
    cd_set_DAC(0);
    ac_spark_detect_disable();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_OFF, common.error_ADC);
    return OFF_STATE;
}

states_t anode_error_handler(void)
{
    ahp_disable();
    anode.PWM_output = 0;
    ap_set_PWM(anode.PWM_output);
    cd_set_DAC(0);
    ac_spark_detect_disable();
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return ERROR_STATE;
}

states_t anode_error_cleared_handler(void)
{
    common.error_code = ANODE_NO_ERROR;
    common.error_ADC = 0;
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return OFF_STATE;
}