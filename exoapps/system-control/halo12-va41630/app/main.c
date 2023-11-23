/**
* @file    main.c
*
* @brief   Main entry point for the Halo 12 production app.
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

#include "task-monitor/task_monitor.h"
#include "task-monitor/component_tasks.h"
#include "definitions.h"

#include "FreeRTOS.h"

#ifndef FREE_RTOS
#define FREE_RTOS
#endif



#ifdef EXORUN_WITH_FIXED_RESID
#ifdef EXORUN_WITH_ECPK_RESID
#define FIXED_RESID ECPK_RESID
#elif defined(EXORUN_WITH_MVCP_RESID)
#define FIXED_RESID MVCP_RESID
#elif defined(EXORUN_WITH_ACP_RESID)
#define FIXED_RESID ACP_RESID
#elif defined(EXORUN_WITH_SYS_CTRL_RESID)
#define FIXED_RESID SYS_CTRL_RESID
#elif defined(EXORUN_WITH_HRD_CTRL_RESID)
#define FIXED_RESID HRD_CTRL_RESID
#elif defined(EXORUN_WITH_ALL_CTRL_RESID)
#define FIXED_RESID ALL_CTRL_RESID
#endif
#endif  // EXORUN_WITH_FIXED_RESID



int main(void)
{
#ifdef EXORUN_WITH_FIXED_RESID
    system_res_id = FIXED_RESID;    // If this is undefined, check the project config to ensure a valid option is enabled
#else
    system_res_id = comp_read_res_id();
#endif

    task_monitor_start();
}
