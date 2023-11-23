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
#include "component_service.h"
#include "valve_control.h"
#include "valve_mcu.h"

static health_table_entry_t health_response_0[] =
    DECLARE_VALVE_HEALTH_ENTRY_0(&valve.tank_pressure,
                                 &valve.cathode_pressure,
                                 &valve.anode_pressure,
                                 &valve.regulator_pressure);

static health_table_entry_t health_response_1[] =
    DECLARE_VALVE_HEALTH_ENTRY_1(&valve.common.current_state,
                                 &valve.common.error_code,
                                 &valve.anode_dac,
                                 &valve.cat_low_flow_dac);

static health_table_entry_t health_response_2[] =
    DECLARE_VALVE_HEALTH_ENTRY_2(&valve.temperature,
                                 &valve.cat_high_flow_voltage);

static health_array valve_health = {
    health_response_0,
    health_response_1,
    health_response_2
};

int vc_sync(message_t *msg)
{
    cc_sync(msg->data, valve_health, COMM_ID_VALVE);
    return 0;
}

void valve_control_init(void)
{
    valve.anode_flow_setpoint = 0;
    valve.cat_high_flow_setpoint = 0;
    valve.cat_low_flow_setpoint = 0;
    valve.common.error_code = VALVE_NO_ERROR;
    valve.common.error_adc = 0;
    valve.cat_low_flow_dac = 0;
    valve.anode_dac = 0;
}

states_t valve_state_get(void)
{
    return valve.common.current_state;
}
