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

#ifndef COMMON_64M1_HV_H
#define	COMMON_64M1_HV_H

#include "adc/hal_adc.h"
#include "anode_adc.h"
#include "anode_command_handlers.h"
#include "anode_control.h"
#include "anode_initialize.h"
#include "anode_state_handlers.h"
#include "anode_timer.h"
#include "component_service.h"
#include "keeper_adc.h"
#include "keeper_command_handlers.h"
#include "keeper_control.h"
#include "keeper_initialize.h"
#include "keeper_state_handlers.h"
#include "magnet_adc.h"
#include "magnet_command_handlers.h"
#include "magnet_control.h"
#include "magnet_initialize.h"
#include "magnet_state_handlers.h"
#include "magnet_timer.h"
#include "valve_adc.h"
#include "valve_command_handlers.h"
#include "valve_control.h"
#include "valve_initialize.h"
#include "valve_state_handlers.h"

// these IDs are just place holders
typedef enum
{
    VALVE_ID          = 0,
    KEEPER_ID         = 1,
    ANODE_ID          = 2,
    MAGNET_ID         = 3,
} MCU_ids_t;

operations_t operation_array[] = 
{
    [VALVE_ID] = {
        .state_handler = {
            [INIT_STATE]    = valve_initialize,
            [OFF_STATE]     = NULL,
            [STARTUP_STATE] = NULL,
            [ON_STATE]      = valve_on_state,
            [ERROR_STATE]   = valve_error_state
        },
        .command_table_get  = valve_command_table_get,
        .control_flag_check = ca_data_ready_check,
        .control_flag_reset = ca_data_ready_reset,
        .data_get           = va_get_data,
    },
    [KEEPER_ID] = {
        .state_handler = {
            [INIT_STATE]    = keeper_initialize,
            [OFF_STATE]     = NULL,
            [STARTUP_STATE] = keeper_startup_state,
            [ON_STATE]      = keeper_on_state,
            [ERROR_STATE]   = keeper_error_state
        },
        .command_table_get  = keeper_command_table_get,
        .control_flag_check = ca_data_ready_check,
        .control_flag_reset = ca_data_ready_reset,
        .data_get           = ka_get_data,
    },
    [ANODE_ID] = {
        .state_handler = {
            [INIT_STATE]    = anode_initialize,
            [OFF_STATE]     = NULL,
            [STARTUP_STATE] = anode_startup_state,
            [ON_STATE]      = anode_on_state,
            [ERROR_STATE]   = anode_error_state
        },
        .command_table_get  = anode_command_table_get,
        .control_flag_check = at_timer_flag_check,
        .control_flag_reset = at_timer_flag_reset,
        .data_get           = aa_get_data,
    },
    [MAGNET_ID] = {
        .state_handler = {
            [INIT_STATE]    = magnet_initialize,
            [OFF_STATE]     = NULL,
            [STARTUP_STATE] = magnet_startup_state,
            [ON_STATE]      = magnet_on_state,
            [ERROR_STATE]   = magnet_error_state
        },
        .command_table_get  = magnet_command_table_get,
        .control_flag_check = mt_timer_flag_check,
        .control_flag_reset = mt_timer_flag_reset,
        .data_get           = ma_get_data,
    }
};

#endif	/* COMMON_64M1_HV_H */

