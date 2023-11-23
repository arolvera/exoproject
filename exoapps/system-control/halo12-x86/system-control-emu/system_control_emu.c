/*
           Copyright (C) 2022 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file via any medium is strictly prohibited.
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/
#include "definitions.h"
#include "task-monitor/task_monitor.h"
#include "task-monitor/component_tasks.h"
#include "sys/sys_timers.h"

#ifndef FREE_RTOS
#define FREE_RTOS
#endif



/* This emulator file should always run with fixed resistor ID */
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



/**
 * @brief   Main entry point for the Halo12 x86 simulator
 * @return  Does not return application quits.
 */
int main(void)
{
    system_res_id = FIXED_RESID;
    sys_timer_w_cb_init();

    task_monitor_start();

    return 0;
}