/**
 * @file:   task_monitor.h
 *
 * @brief:  This is the first task that starts after main. Task monitor is responsible for:
 * - Initial hardware init and peripheral init.
 * - sys_init
 * - Initialize msg_handler
 * - Starting and stopping tasks
 * - Monitors task running
 *
 * @copyright   Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef TASK_MONITOR_
#define TASK_MONITOR_

/**
 * @brief   Start the task monitor.
 *
 * This function initializes the system and starts up appropriate tasks
 * according to build options.
 */
void task_monitor_start(void);


#endif //TASK_MONITOR_
