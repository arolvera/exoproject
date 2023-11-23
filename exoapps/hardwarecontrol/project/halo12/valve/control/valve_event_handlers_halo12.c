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
#include "dac/hal_dac.h"
#include "valve_event_handlers.h"
#include "valve_hardware_port.h"
#include "valve_mcu.h"
#include "valve_pwm.h"
#include "valve_state_handlers.h"

states_t valve_on_command_handler(void)
{
    vhp_lv_open();
    vsh_reset_counters();
    cc_state_change_report(ON_STATE, COMMANDED_ON, COMM_ID_VALVE);
    return ON_STATE;
}

states_t valve_off_command_handler(void)
{
    vhp_lv_close();
    vp_set_pwm(0);
    valve.cat_low_flow_dac = 0;
    dac_set(valve.cat_low_flow_dac, CATHODE_LF_DAC);
    valve.anode_dac = 0;
    dac_set(valve.anode_dac, ANODE_FLOW_DAC);
    cc_state_change_report(OFF_STATE, COMMANDED_OFF, COMM_ID_VALVE);
    return OFF_STATE;
}

states_t valve_error_handler(void)
{
    vhp_lv_close();
    vp_set_pwm(0);
    valve.cat_low_flow_dac = 0;
    dac_set(valve.cat_low_flow_dac, CATHODE_LF_DAC);
    valve.anode_dac = 0;
    dac_set(valve.anode_dac, ANODE_FLOW_DAC);
    cc_error_report(valve.common.error_code, valve.common.error_adc, COMM_ID_VALVE);
    cc_state_change_report(ERROR_STATE, ERROR_DETECTED, COMM_ID_VALVE);
    return ERROR_STATE;
}

states_t valve_error_cleared_handler(void)
{
    valve.common.error_code = VALVE_NO_ERROR;
    valve.common.error_adc = 0;
    cc_error_report(valve.common.error_code, valve.common.error_adc, COMM_ID_VALVE);
    cc_state_change_report(OFF_STATE, ERROR_CLEARED, COMM_ID_VALVE);
    return OFF_STATE;
}