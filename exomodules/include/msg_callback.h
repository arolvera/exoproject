/**
 * @file    msg_callback.h
 *
 * @brief   Data structures for the message handler.
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

#ifndef MSG_CALLBACK_H
#define MSG_CALLBACK_H

#include "binary_range_tree.h"

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */
//Move to common location
typedef struct message_t_ {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
} message_t;
#pragma pack(pop)

typedef int (msg_callback)(message_t *);

typedef struct msg_callback_t_ {
    range_node_t node;
    msg_callback *cb;
} msg_callback_t;


#endif//MSG_CALLBACK_H
