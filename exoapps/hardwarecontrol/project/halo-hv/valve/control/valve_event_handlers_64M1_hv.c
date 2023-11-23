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
#include "valve_control.h"
#include "valve_event_handlers.h"
#include "valve_hardware_port.h"
#include "valve_pwm.h"
#include "valve_state_handlers.h"
#include <stddef.h>

states_t valve_on_command_handler(void)
{
    vhp_lv_open();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_ON, common.error_ADC);
    return ON_STATE;
}

states_t valve_off_command_handler(void)
{
    vhp_lv_close();
    vp_set_PWM(0, CATHODE_HIGH_FLOW_VALVE);
    valve.cat_low_flow_PWM = 0;
    vp_set_PWM(valve.cat_low_flow_PWM, CATHODE_LOW_FLOW_VALVE);
    valve.anode_PWM = 0;
    vp_set_PWM(valve.anode_PWM, ANODE_FLOW_VALVE);
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_OFF, common.error_ADC);
    return OFF_STATE;
}

states_t valve_error_handler(void)
{
    vhp_lv_close();
    vp_set_PWM(0, CATHODE_HIGH_FLOW_VALVE);
    valve.cat_low_flow_PWM = 0;
    vp_set_PWM(valve.cat_low_flow_PWM, CATHODE_LOW_FLOW_VALVE);
    valve.anode_PWM = 0;
    vp_set_PWM(valve.anode_PWM, ANODE_FLOW_VALVE);
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return ERROR_STATE;
}

states_t valve_error_cleared_handler(void)
{
    common.error_code = VALVE_NO_ERROR;
    common.error_ADC = 0;
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return OFF_STATE;
}