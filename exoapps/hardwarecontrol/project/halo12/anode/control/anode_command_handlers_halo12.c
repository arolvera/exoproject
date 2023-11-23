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

#include "anode_command_handlers.h"
#include "anode_control.h"
#include "anode_event_handlers.h"
#include "anode_mcu.h"
#include "component_communication.h"

static void on_off(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point == ON_SET_POINT && anode.common.current_state == OFF_STATE)
    {
        anode.common.current_state = anode_on_command_handler();
    }
    else if(msg->command.set_point == OFF_SET_POINT && anode.common.current_state != ERROR_STATE)
    {
        anode.common.current_state = anode_off_command_handler();
    } else {
        cc_error_report(INVALID_STATE, 0, COMM_ID_ANODE);
    }
}

static void set_voltage(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point < ANODE_MAX_V_OUT_COUNTS && msg->command.set_point > ANODE_MIN_V_OUT_COUNTS)
    {
        anode.target_voltage = msg->command.set_point;
    } else {
        cc_error_report(INVALID_SETPOINT, 0, COMM_ID_ANODE);
    }
}

static command_function_t anode_commands[] = 
{
    {
        ON_OFF, on_off
    },
    {
        SET_VOLTAGE, set_voltage
    },
    {
        INVALIDATE_IMG, cs_invalidate_img
    },
    {
        PROCESSOR_RESET, cs_reset_proc,
    },

};

#define ANODE_COMMAND_TABLE_SIZE sizeof(anode_commands)/sizeof(command_function_t)

static command_table_t anode_command_table =
{
    anode_commands,
    ANODE_COMMAND_TABLE_SIZE
};

command_table_t* anode_command_table_get(void)
{
    return &anode_command_table;
}