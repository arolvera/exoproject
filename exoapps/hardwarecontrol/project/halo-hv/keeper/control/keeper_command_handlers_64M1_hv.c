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
#include "keeper_command_handlers.h"
#include "keeper_control.h"
#include "keeper_event_handlers.h"

static void on_off(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point == ON_SET_POINT && common.current_state == OFF_STATE)
    {
        common.current_state = keeper_on_command_handler();
    }
    else if(msg->command.set_point == OFF_SET_POINT && common.current_state != ERROR_STATE)
    {
        common.current_state = keeper_off_command_handler();
    } else {
        cc_invalid_state();
    }
}

static void set_current(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point < KEEPER_MAX_I_OUT_COUNTS)
    {
        keeper.target_current = msg->command.set_point;
    } else {
        cc_invalid_setpoint();
    }
}

static void set_voltage(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point < KEEPER_MAX_V_OUT_COUNTS)
    {
        keeper.target_voltage = msg->command.set_point;
    } else {
        cc_invalid_setpoint();
    }
}

static command_function_t keeper_commands[] = 
{
    {
        ON_OFF, on_off
    },
    {
        SET_CURRENT, set_current
    },
    {
        SET_VOLTAGE, set_voltage
    }
};

#define KEEPER_COMMAND_TABLE_SIZE sizeof(keeper_commands)/sizeof(command_function_t)

static command_table_t keeper_command_table =
{
    keeper_commands,
    KEEPER_COMMAND_TABLE_SIZE
};

command_table_t* keeper_command_table_get(void)
{
    return &keeper_command_table;
}