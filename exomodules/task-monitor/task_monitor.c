/**
 * @file:   task_monitor.c
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

#include "task_monitor.h"
#include "msg-handler/msg_handler.h"
#include "task-monitor/component_tasks.h"
#include "sys_init.h"
#if BUILD_PERCEPIO
#include "trcRecorder.h"
#endif

void task_monitor_start(void)
{
    const comp_task_list_t *comp_list;
    comp_task_t *comp;

    //sys_init
    comp_pre_init();

#if BUILD_PERCEPIO
    xTraceEnable(TRC_START);
    traceString chn = xTraceRegisterString("Start");
    vTracePrint(chn, "Starting task mon");
#endif

    //Init msg handler task and can_bus
    msg_handler_init(0);


    comp_list = comp_task_list_get();
    //Init enabled tasks
    for(int i = 0; i < comp_list->num_comp_tasks; i++) {
        comp = comp_list->comp_tasks[i];
        if(comp->comp_func.comp_task_init) {
            comp->comp_func.comp_task_init();
        }
    }

    //Start enabled tasks
    for(int i = 0; i < comp_list->num_comp_tasks; i++) {
        comp = comp_list->comp_tasks[i];
        if(comp->comp_info.start_on_boot && comp->comp_func.comp_task_start) {
            //Maybe add pcName for percepio
            comp->comp_task_stack.task_handle =
                    OSAL_TASK_CreateStatic(
                    comp->comp_func.comp_task_start,
                    comp->comp_info.comp_name,
                    comp->comp_task_stack.task_stack_size,
                    &comp->comp_info.comm_id,
                    comp->comp_task_stack.task_prio,
                    comp->comp_task_stack.task_stack,
                    &comp->comp_task_stack.task_buf
                                 );
        }
    }

    vTaskStartScheduler();

    for(;;) {
    }
}
