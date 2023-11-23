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

#include "common_ADC.h"
#include "component_communication.h"
#include "component_service.h"
#include "valve_adc.h"
#include "valve_command_handlers.h"
#include "valve_control.h"
#include "valve_mcu.h"
#include "valve_state_handlers.h"
#include <stddef.h>

static health_table_entry_t health_response_0[] = 
    DECLARE_VALVE_HEALTH_ENTRY_0(&valve.anode_flow_voltage, 
                                 &valve.cat_high_flow_voltage, 
                                 &valve.cat_low_flow_voltage,
                                 &valve.temperature);

static health_table_entry_t health_response_1[] = 
    DECLARE_VALVE_HEALTH_ENTRY_1(&valve.tank_pressure,
                                 &valve.cathode_pressure,
                                 &valve.anode_pressure,
                                 &valve.regulator_pressure);

static health_array valve_health = {
    health_response_0,
    health_response_1
};

void vc_sync(uint32_t id, uint8_t dlc, volatile uint8_t *fifo)
{
    cc_sync(CAN_ID_VALVE, dlc, fifo, valve_health);
}

void valve_control_init(void)
{
    valve.anode_flow_setpoint = 0;
    valve.cat_high_flow_setpoint = 0;
    valve.cat_low_flow_setpoint = 0;
    common.error_code = VALVE_NO_ERROR;
    common.error_ADC = 0;
}
