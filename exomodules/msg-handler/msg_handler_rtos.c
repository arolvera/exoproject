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
#include "FreeRTOS.h"
#include "queue.h"
#include "msg_handler.h"
#include "task-monitor/component_tasks.h"
#include "mcu_include.h"
#include "task_priority.h"
#include "trace/trace.h"
#include "utils/macro_tools.h"
#include "component_callback.h"
#include "cmsis/cmsis_gcc.h"

#define QUEUE_ITEM_SIZE (sizeof(message_t))
#define QUEUE_BUFFER_COUNT 10
static StaticQueue_t msg_handler_queue_rx;
static QueueHandle_t msg_handler_queue_rx_handle;
static uint8_t msg_queue_storage_area[QUEUE_ITEM_SIZE * QUEUE_BUFFER_COUNT];

#define MSG_HANDLER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 4)
static TaskHandle_t msg_handler_task = NULL;
static StackType_t msg_handler_task_stack[MSG_HANDLER_TASK_STACK_SIZE];
static StaticTask_t msg_handler_task_buffer;

static range_node_t *can_callback_tree = NULL;

_Noreturn static void msg_handler_recv_task(void *pvParameters);

static int can_handle;

void msg_handler_init(int comm_id)
{
    OSAL_QUEUE_CreateStatic(&msg_handler_queue_rx_handle, QUEUE_BUFFER_COUNT, QUEUE_ITEM_SIZE, msg_queue_storage_area,
                           &msg_handler_queue_rx, "msg_hndl_q");
    //Init shared queue
    xTaskCreateStatic((TaskFunction_t)msg_handler_recv_task,
                      "msg_hndl",
                      MSG_HANDLER_TASK_STACK_SIZE,
                      &msg_handler_task,
                      MSG_HANDLER_TASK_PRIO,
                      msg_handler_task_stack,
                      &msg_handler_task_buffer);

    //Init can
    can_handle = comp_task_can_init(&msg_handler_queue_rx_handle);
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
    message_t msg = {.id = id, .dlc = dlc, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
    if(data != NULL) {
        msg.id = id;
        msg.dlc = dlc;
        memcpy(&msg.data, data, dlc);
        //Get comm interface to send msg over
        switch(comp_task_get_intf((msg.id))) {
            case INTF_CAN:
                err = can_send(can_handle, &msg, timeout);
                break;
            case INTF_QUEUE:
                err = ~xQueueSend(msg_handler_queue_rx_handle, &msg, timeout);
                break;
            case INTF_ALL:
                err = can_send(can_handle, &msg, timeout);
                err = ~xQueueSend(msg_handler_queue_rx_handle, &msg, timeout);
                break;
            case INTF_INVALID:
                err = -1;
                break;
        }
    }
    return err;
}

_Noreturn static void msg_handler_recv_task(void *pvParameters)
{
    UBaseType_t err_rx;
    message_t message_rx;
    range_node_t *cur_node;

    for(;;) {
        // Check for any needed action from the rx queue
        err_rx = xQueueReceive(msg_handler_queue_rx_handle, &message_rx, portMAX_DELAY);

        if(err_rx == pdTRUE) {
            cur_node = binary_range_search(can_callback_tree, message_rx.id);
            if(cur_node != NULL) {
                msg_callback_t *cb = container_of(cur_node, msg_callback_t, node);
                cb->cb(&message_rx);
            }
        }
    }
}

/**
 * Register a callback function for a given ranges of message IDs
 * @param cb_node pointer to callback node
 */
void msg_handler_register_callback(msg_callback_t *cb_node)
{
    range_node_t *result = binary_range_search(can_callback_tree, cb_node->node.range_high);
    if(result == NULL) {
        can_callback_tree = binary_range_insert(can_callback_tree, &cb_node->node);
    }
    if(can_callback_tree == NULL) {
        /* Programming error - Overlapping ranges perhaps? */
        TraceE1(TrcMsgErr1, "ERORR: msg id high:0x%x low:0x%x",
                cb_node->node.range_high, cb_node->node.range_low, 0, 0, 0, 0);
#if defined(__DEBUG) || defined(__DEBUG_D)
        __BKPT(0);
#endif
    }
}
