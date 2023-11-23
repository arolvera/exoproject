/**
 * @file    msg_callback.h
 *
 * @brief   An abstraction layer for communication between software components.
 *
 * How is it that both BM and RTOS versions appear to be included in the cmake config?
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef MSG_HANDLER_H
#define MSG_HANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "hal.h"
#include "msg_callback.h"



/**
 * @brief
 * @param comm_id
 */
void msg_handler_init(int comm_id);

/**
 * @brief
 * @param id
 * @param data
 * @param dlc
 * @param timeout
 * @return
 */
int send_msg(uint16_t id, uint8_t *data, can_dlc_t dlc, uint32_t timeout);

/**
 * @brief
 * @param message
 * @param timeout
 * @return
 */
int recv_msg(message_t *message, uint32_t timeout);

/**
 * @brief
 * @param cb_node
 */
void msg_handler_register_callback(msg_callback_t *cb_node);



#ifdef	__cplusplus
}
#endif

#endif//MSG_HANDLER_H
