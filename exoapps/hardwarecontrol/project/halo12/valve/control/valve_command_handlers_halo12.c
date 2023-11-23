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
#include "valve_command_handlers.h"
#include "valve_control.h"
#include "valve_event_handlers.h"
#include "valve_mcu.h"

static void on_off(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point == ON_SET_POINT && valve.common.current_state == OFF_STATE)
    {
        valve.common.current_state = valve_on_command_handler();
    }
    else if(msg->command.set_point == OFF_SET_POINT && valve.common.current_state != ERROR_STATE)
    {
        valve.common.current_state = valve_off_command_handler();
    } else {
        cc_error_report(INVALID_STATE, 0, COMM_ID_VALVE);
    }
}

static void set_flow(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.specifier == CAT_HIGH_FLOW)
    {
        if(msg->command.set_point <= VALVE_MAX_HIGH_FLOW)
        {
            valve.cat_high_flow_setpoint = msg->command.set_point;
        } else {
            cc_error_report(INVALID_SETPOINT, 0, COMM_ID_VALVE);
        }
    }
    else if(msg->command.specifier == CAT_LOW_FLOW)
    {
        if(msg->command.set_point <= VALVE_MAX_PRESSURE_COUNTS)
        {
            valve.cat_low_flow_setpoint = msg->command.set_point;
        } else {
            cc_error_report(INVALID_SETPOINT, 0, COMM_ID_VALVE);
        }
    }
    else if(msg->command.specifier == ANODE_FLOW)
    {
        if(msg->command.set_point <= VALVE_MAX_PRESSURE_COUNTS)
        {
            valve.anode_flow_setpoint = msg->command.set_point;
        } else {
            cc_error_report(INVALID_SETPOINT, 0, COMM_ID_VALVE);
        }
    } else {
        cc_error_report(INVALID_COMMAND, 0, COMM_ID_VALVE);
    }
}

static command_function_t valve_commands[] = 
{
    {
        ON_OFF, on_off
    },
    {
        SET_FLOW, set_flow
    }
};

#define VALVE_COMMAND_TABLE_SIZE sizeof(valve_commands)/sizeof(command_function_t)

static command_table_t valve_command_table =
{
    valve_commands,
    VALVE_COMMAND_TABLE_SIZE
};

command_table_t* valve_command_table_get(void)
{
    return &valve_command_table;
}