/**
 * @file    sequence_test.c
 *
 * @brief   Main entry point for the sequence engine test app. I originally that
 * the sequence engine could be tested in isolation but it is looking to be too
 * coupled with other code. Consequently, this app may not look any different
 * than the main system control application.
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

#include "definitions.h"
#include "FreeRTOS.h"
#include "task-monitor/task_monitor.h"
#include "task-monitor/component_tasks.h"

#ifndef FREE_RTOS
#define FREE_RTOS
#endif



/**
 * @brief   Main entry point for the sequence engine test application.
 * @return  Does not return application quits.
 */
int main(void)
{
    system_res_id = ECPK_RESID;

    task_monitor_start();

    return 0;
}
