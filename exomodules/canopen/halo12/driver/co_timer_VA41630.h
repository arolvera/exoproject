/**
 * @file    co_timer_VA41630.h
 *
 * @brief   ??? Defines an extern timer driver object. This is a container for init,
 * reload, delay, stop, start, and update functions statically defined in the c file.
 *
 * What does canopen do with the timer???? Used for heartbeat and sdo timeouts.
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

#ifndef CO_TIMER_VA41630_H
#define CO_TIMER_VA41630_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_if.h"


/* ToDo: rename ATSAMV71 to VA41630 */
extern const CO_IF_TIMER_DRV ATSAMV71TimerDriver;



#ifdef __cplusplus               /* for compatibility with C++ environments  */
}
#endif

#endif  // CO_TIMER_VA41630_H
