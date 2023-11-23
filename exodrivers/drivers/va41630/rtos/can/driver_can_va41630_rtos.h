// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
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

#ifndef _CAN_DRIVER_RTOS_H
#define _CAN_DRIVER_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can/hal_can.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


int can_notify_rx_task_RTOS(void* arg);
int can_queue_rx_message_RTOS(int handle, message_t *msg);
int can_notif_tx_buff_avail_RTOS(void* arg);


#ifdef __cplusplus
}
#endif

#endif