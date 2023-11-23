/**
 * @file    co_task.h
 *
 * @brief   ??? CanOpen task functions for NMT
 *
 * Example program to evaluate the CAN open stack (clock_task.h)
 * Really? Is that a copy and paste that was never cleaned up?
 *
 * If comm redundancy requires multiple can open nodes, should we create two
 * extern node variables here, each a container for its own init, start, etc.
 * functions? We are doing one node.
 *
 * Is the implementation complete? Anything to do? Why is canopen_task_nmt_preop
 * commented out in app.c? Delete this here and app.c
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

#ifndef CAN_OPEN_TASK_H
#define CAN_OPEN_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_core.h"
#include "co_obj.h"



void co_task_init(void);
void canopen_task_node_start(void);
void canopen_task_nmt_error_state(void);
void canopen_task_nmt_preop(void);
void canopen_task_nmt_op(void);
void canopen_task_enable_heartbeat(void);
void canopen_task_send_hb(void);



#ifdef __cplusplus
}
#endif

#endif /* CAN_OPEN_TASK_H */

