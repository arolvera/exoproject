//
// Created by marvin on 1/4/23.
//
//
//            Copyright (C) 2022 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com
//
#include "msg_handler.h"
#include "mcu_include.h"
#include "utils/macro_tools.h"
#include "can/hal_can.h"
#include "cmsis/cmsis_gcc.h"

static int can_handle;
static range_node_t *can_callback_tree = NULL;

void msg_handler_init(int comm_id)
{
    can_rx_buffer_t can_rx_buf[] = {{.rx_mob_count = 2,
        .filter_type = CAN_FILTER_ID,
        .filter_high = (COMMAND_PARAMETERS_ID_BASE | comm_id),
        .filter_low = (COMMAND_PARAMETERS_ID_BASE | comm_id)},
        {.rx_mob_count = 1,
            .filter_type = CAN_FILTER_ID,
            .filter_high = HSI_ID,
            .filter_low = HSI_ID}};

    can_init_t can_ini = {.baud = CAN_BAUD_RATE_1000,
        .tx_mob_count = 10,
        .rx_buffers = can_rx_buf,
        .rx_buffer_len = 2,
        .irq_priority = 0};

    can_handle = can_init(&can_ini);
}

/**
 * Sends message to through an interface given a message
 * @param message - msg to send
 * @param iface - enum for msg_handler to choose which interface to use
 * @return
 */
int send_msg(uint16_t id, uint8_t *data, can_dlc_t dlc, uint32_t timeout)
{
    int err = -1;
    message_t msg = {0};
    msg.id = id;
    msg.dlc = dlc;
    memcpy(&msg.data, data, dlc);
    err = can_send(can_handle, &msg, timeout);
    return err;
}

int recv_msg(message_t *msg, uint32_t timeout)
{
    bool msg_ready = false;
    can_rx_service(can_handle);
    range_node_t *curr_node;
    if(!can_rx_is_empty(can_handle)){
        can_rcv(can_handle, msg, 1000);

        if(msg->id == HSI_ID){
            curr_node = binary_range_search(can_callback_tree, msg->id);

            if(curr_node){
                msg_callback_t *cb = container_of(curr_node, msg_callback_t, node);
                cb->cb(msg);
            }else{
                msg_ready = false;
            }
        } else {
            msg_ready = true;
        }
    }
    return msg_ready;
}

/**
 * Register a callback function for a given ranges of message IDs
 * @param cb_node pointer to callback node
 */
void msg_handler_register_callback(msg_callback_t *cb_node)
{
    can_callback_tree = binary_range_insert(can_callback_tree, &cb_node->node);
    if(can_callback_tree == NULL) {
        /* Programming error - Overlapping ranges perhaps? */
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __BKPT(0);
#endif
    }
}
