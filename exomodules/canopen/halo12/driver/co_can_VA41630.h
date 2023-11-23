/**
 * @file    co_can_VA41630.h
 *
 * @brief   ??? Defines an extern canopen driver object. This is a container for init,
 * enable, read, send, reset, and close functions statically defined in the c file.
 *
 * CanOpen communications between the ECPK and FCP occur over RS485 which connects
 * to a UART. Which one? I can't see where uart_init_t is initialized.
 * UART 0 is used for now for dev board. We will use 1 and 2 for redundant comms on ECPK
 *
 * The implementation looks to be partial. What was the plan?
 * Close and reset need to remain as nop functions.
 *
 * To support redundancy, do we need parallel canopen nodes pointed at separate ports?
 * Or can we have one node that combines each port in every function (init, send, etc.)?
 * Prefer one node. Get clarification on use case.
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

#ifndef CO_CAN_VA41630_H
#define CO_CAN_VA41630_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_if.h"



/* TODO: rename the extern variable declaration to match the naming convention:
 *   <device-name>CanDriver
 */
extern const CO_IF_CAN_DRV CanOpenDriver;



#ifdef __cplusplus               /* for compatibility with C++ environments  */
}
#endif

#endif  // CO_CAN_VA41630_H
