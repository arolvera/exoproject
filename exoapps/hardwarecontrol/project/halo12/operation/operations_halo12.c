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

#ifndef OPERATIONS_HALO12_H
#define    OPERATIONS_HALO12_H

#include "anode_adc.h"
#include "anode_command_handlers.h"
#include "anode_control.h"
#include "anode_initialize.h"
#include "anode_state_handlers.h"
#include "anode_timer.h"
#include "keeper_adc.h"
#include "keeper_command_handlers.h"
#include "keeper_control.h"
#include "keeper_initialize.h"
#include "keeper_state_handlers.h"
#include "keeper_timer.h"
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
#include "valve_timer.h"
#ifndef SINGLE_COMPONENT_BUILD
#include "component_callback.h"
#endif
#define DECLARE_GLOBALS
#include "ext_decl_define.h"
#include "operations.h"

#ifndef SINGLE_COMPONENT_BUILD
static msg_callback_t keeper_cb = {
    .cb = comp_cmd_cb,
    .node = {
        .range_low = COMMAND_PARAMETERS_ID_KEEPER,
        .range_high = COMMAND_PARAMETERS_ID_KEEPER,
        .left = NULL,
        .right = NULL
    }
};

static msg_callback_t anode_cb = {
    .cb = comp_cmd_cb,
    .node = {
        .range_low = COMMAND_PARAMETERS_ID_ANODE,
        .range_high = COMMAND_PARAMETERS_ID_ANODE,
        .left = NULL,
        .right = NULL
    }
};

static msg_callback_t magent_cb = {
    .cb = comp_cmd_cb,
    .node = {
        .range_low = COMMAND_PARAMETERS_ID_MAGNET,
        .range_high = COMMAND_PARAMETERS_ID_MAGNET,
        .left = NULL,
        .right = NULL
    }
};

static msg_callback_t valve_cb = {
    .cb = comp_cmd_cb,
    .node = {
        .range_low = COMMAND_PARAMETERS_ID_VALVE,
        .range_high = COMMAND_PARAMETERS_ID_VALVE,
        .left = NULL,
        .right = NULL
    }
};
#endif //SINGLE_COMPONENT_BUILD

operations_t operations[] =
    {
        [KEEPER_ID] = {
            .state_handler = {
                [INIT_STATE]    = keeper_initialize,
                [OFF_STATE]     = NULL,
                [STARTUP_STATE] = keeper_startup_state,
                [ON_STATE]      = keeper_on_state,
                [ERROR_STATE]   = keeper_error_state},
            .command_table_get  = keeper_command_table_get,
            .current_state_get  = keeper_state_get,
            .control_flag_check = kt_timer_flag_check,
            .control_flag_reset = kt_timer_flag_reset,
            .control_sync       = kc_sync,
            .data_get           = ka_get_data,
            .communication_id   = COMM_ID_KEEPER,
#ifndef SINGLE_COMPONENT_BUILD
            .cb = &keeper_cb,
#endif //SINGLE_COMPONENT_BUILD
        },
        [ANODE_ID] = {
            .state_handler = {
                [INIT_STATE]    = anode_initialize,
                [OFF_STATE]     = NULL,
                [STARTUP_STATE] = anode_startup_state,
                [ON_STATE]      = anode_on_state,
                [ERROR_STATE]   = anode_error_state},
            .command_table_get  = anode_command_table_get,
            .current_state_get  = anode_state_get,
            .control_flag_check = at_timer_flag_check,
            .control_flag_reset = at_timer_flag_reset,
            .control_sync       = ac_sync,
            .data_get           = aa_get_data,
            .communication_id   = COMM_ID_ANODE,
#ifndef SINGLE_COMPONENT_BUILD
            .cb = &anode_cb,
#endif //SINGLE_COMPONENT_BUILD
        },
        [MAGNET_ID] = {
            .state_handler = {
                [INIT_STATE]    = magnet_initialize,
                [OFF_STATE]     = NULL,
                [STARTUP_STATE] = magnet_startup_state,
                [ON_STATE]      = magnet_on_state,
                [ERROR_STATE]   = magnet_error_state},
            .command_table_get  = magnet_command_table_get,
            .current_state_get  = magnet_state_get,
            .control_flag_check = mt_timer_flag_check,
            .control_flag_reset = mt_timer_flag_reset,
            .control_sync       = mc_sync,
            .data_get           = ma_get_data,
            .communication_id   = COMM_ID_MAGNET,
#ifndef SINGLE_COMPONENT_BUILD
            .cb = &magent_cb,
#endif //SINGLE_COMPONENT_BUILD
        },
        [VALVE_ID] = {
            .state_handler = {
                [INIT_STATE]    = valve_initialize,
                [OFF_STATE]     = NULL,
                [STARTUP_STATE] = NULL,
                [ON_STATE]      = valve_on_state,
                [ERROR_STATE]   = valve_error_state},
            .command_table_get  = valve_command_table_get,
            .current_state_get  = valve_state_get,
            .control_flag_check = vt_timer_flag_check,
            .control_flag_reset = vt_timer_flag_reset,
            .control_sync       = vc_sync,
            .data_get           = va_get_data,
            .communication_id   = COMM_ID_VALVE,
#ifndef SINGLE_COMPONENT_BUILD
            .cb = &valve_cb,
#endif //SINGLE_COMPONENT_BUILD
        },
    };


#endif    /* OPERATIONS_HALO12_H */

