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
#include "magnet_command_handlers.h"
#include "magnet_event_handlers.h"
#include "magnet_mcu.h"

static void on_off(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point == ON_SET_POINT && magnet.common.current_state == OFF_STATE)
    {
        magnet.common.current_state = magnet_on_command_handler();
    }
    else if(msg->command.set_point == OFF_SET_POINT && magnet.common.current_state != ERROR_STATE)
    {
        magnet.common.current_state = magnet_off_command_handler();
    } else {
        cc_error_report(INVALID_STATE, 0, COMM_ID_MAGNET);
    }
}

static void set_current(uint8_t *data)
{
    communication_union_t *msg = (communication_union_t*)data;
    if(msg->command.set_point < MAGNET_MAX_I_OUT_COUNTS)
    {
        magnet.target_current = msg->command.set_point;
    } else {
        cc_error_report(INVALID_SETPOINT, 0, COMM_ID_MAGNET);
    }
}

static command_function_t magnet_commands[] = 
{
    {
        ON_OFF, on_off
    },
    {
        SET_CURRENT, set_current
    },
    {
            INVALIDATE_IMG, cs_invalidate_img
    },
    {
            PROCESSOR_RESET, cs_reset_proc,
    },
};

#define MAGNET_COMMAND_TABLE_SIZE sizeof(magnet_commands)/sizeof(command_function_t)

static command_table_t magnet_command_table =
{
    magnet_commands,
    MAGNET_COMMAND_TABLE_SIZE
};

command_table_t* magnet_command_table_get(void)
{
    return &magnet_command_table;
}