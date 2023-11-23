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

/**
 * Basic Explanation:
 * The electronics all share a common state machine. There are five states: 
 * INIT, OFF, STARTUP, ON, and ERROR. It isn't necessary to use all those 
 * states, but no additional states can be added. All high level control happens
 * in either state handlers or event handlers. State handlers are run over and
 * over and handle the moment-to-moment control. Event handlers are only called
 * when there is an event, like an ON command or a comparator interrupt. State
 * changes are caused by events. Events always return states, so whenever an
 * event handler is called, the syntax should look like:
 * 
 * common.current_state = generic_event();
 * 
 * All hardware control must happen in state handlers or event handlers. 
 * Interrupts and command handlers should never control hardware directly, they 
 * should instead call event handlers that control hardware. It may sometimes be
 * inconvenient to do things this way, but it means all functionality and the 
 * entire state machine can be understood just from the event and state 
 * handlers. No important control can ever be forgotten in an ISR.
 */


#ifndef SINGLE_COMPONENT_BUILD
#include "task-monitor/component_tasks.h"
#include "component_callback.h"
#else
#include "msg-handler/msg_handler.h"
#include "component_callback.h"

#endif //SINGLE_COMPONENT_BUILD
#include "operations.h"
#include "component_communication.h"
#include "component_service.h"
#include "ext_decl_define.h"
#define DECLARE_GLOBALS
#include "anode_mcu.h"
#include "keeper_mcu.h"
#include "magnet_mcu.h"
#include "valve_mcu.h"
#include "fram/fram_va41630_cmn.h"
#include "storage/memory-component/halo12/storage/storage_memory_layout.h"
#include "string.h"
#include "device.h"
#include "osal/osal.h"


void cs_init(uint8_t ops_id)
{
    //Init globals
    (void)anode;
    (void)keeper;
    (void)magnet;
    (void)valve;

#ifdef SINGLE_COMPONENT_BUILD
    static msg_callback_t bcast_health = {
        .node = {
            .range_low  = HSI_ID,
            .range_high = HSI_ID,
            .left = NULL,
            .right = NULL,
        },
    };
    bcast_health.cb = operations[ops_id].control_sync,
    msg_handler_register_callback(&bcast_health);
    msg_handler_init(opsid_2_commid(ops_id));
#else //SINGLE_COMPONENT_BUILD
    msg_handler_register_callback(operations[ops_id].cb);
#endif //SINGLE_COMPONENT_BUILD

    (operations[ops_id].state_handler[INIT_STATE])();

}

states_t cs_init_complete(uint8_t ops_id)
{
    cc_state_change_report(OFF_STATE, INIT_COMPLETE, ops_id);
    return OFF_STATE;
}

/**
 * This is the central generic function that runs the generic state machine. 
 * First it looks for a message, and if there is one processes it. Then it 
 * checks if the provided control flag has been set (usually either a timer flag 
 * or an ADC data ready flag, but could be anything). Then it gets fresh data 
 * and runs the state handler for the current state. Finally it resets the 
 * provided control flag.
 */
void cs_service(uint8_t ops_id)
{
#if SINGLE_COMPONENT_BUILD
    message_t msg = {0};
    bool msg_ready = recv_msg(&msg, 1000);
    if(msg_ready)
    {
        if(strcmp((char*)msg.data, "bensgr8") == 0){
            msg.data[0] = INVALIDATE_IMG;
        }
        cc_command_processor(operations[ops_id].command_table_get(), msg.data, operations[ops_id].communication_id);
    }
#endif //SINGLE_COMPONENT_BUILD

    // check if the provided control flag has been set
    if(operations[ops_id].control_flag_check())
    {
        // if a data_get function has been given, run it
        if(operations[ops_id].data_get != NULL)
        {
            operations[ops_id].data_get();
        }

        // get the current state
        states_t current_state = operations[ops_id].current_state_get();

        // check if the current state is valid (never run the init state here)
        if (current_state < NUM_VALID_STATES &&
                current_state != INIT_STATE &&
                operations[ops_id].state_handler[current_state] != NULL)
        {
            // run the provided state handler
            (operations[ops_id].state_handler[current_state])();
        }
        
        // reset flag after running control
        operations[ops_id].control_flag_reset();
    }
}


void cs_invalidate_img(uint8_t* msg)
{
    char invalid_msg[] = "invalid";
    fram_write(APP_START_ADDRESS, (uint8_t*)invalid_msg, sizeof(invalid_msg));
}

void cs_reset_proc(uint8_t* arg)
{
    (void)arg;
    NVIC_SystemReset();
}