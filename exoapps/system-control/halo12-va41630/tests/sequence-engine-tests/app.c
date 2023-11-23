/**
* @file    app.c
*
* @brief   Initialization.
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

/* Allocate the memory for all globals defined in thruster_control.h */
#define DECLARE_GLOBALS
#include "thruster_control.h"
#include "hsi_memory.h"
#undef DECLARE_GLOBALS

#include "app.h"
#include "canopen/co_task.h"
#include "client-control/client_control.h"
#include "client-control/power/client_power.h"
#include "sequence/control_sequence.h"
#include "definitions.h"
//#include "error/error_handler.h"
#include "health/health.h"
#include "sys/sys_timers.h"
#include "task-monitor/component_tasks.h"
#include "trace/trace.h"





#define DETAIL_LOG_LENGTH LOG_LENGTH_16



typedef struct app_error_detail{
    base_error_detail_t b_d;
    app_update_status_t app_update_status;
} app_error_detail_t;

static app_error_detail_t error_detail[DETAIL_LOG_LENGTH];

typedef enum
{
    /* Application's state machine's initial state. */
    APP_STATE_INIT=0,
    APP_STATE_SERVICE_TASKS
} APP_STATES;

static APP_STATES app_state;



static void* app_error_detail_strategy(void* arg)
{
    if(arg != NULL){
        EH_LOG_ERROR(app_update_status, arg);
    }

    return eh_get(MODULE_NUM_APP, APP_SUBMODULE)->error_log;
}

static void app_error_handler_init(void)
{
    eh_create(MODULE_NUM_APP, APP_SUBMODULE, app_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);
//    fh_fault_handlers_register(ERROR_CODE_ECP_INSTALL_ERROR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_BACKUP_RESET, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_WATCHDOG_TIMEOUT, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_NRST, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_UNKNOWN_RESET, FH_ALERT);
}

static void app_state_init()
{
    //    RSTC_CallbackRegister(rstc_callback, 0);

    /* First thing, make sure all clients are off */
    if(system_res_id == ECPK_RESID) {
        cp_power_set(false);
    }

    sys_timer_init();

    /* Init error handling */
    eh_init();

    /* init app errors */
    app_error_handler_init();

    //Trace init
    trcInit();

    //    iacm_init();

    //Storage module init
    //    sm_init();

    //    sys_adc_init_service();

    /* Init client control error handling */
    client_init();

    /* Init canopen emcy nodes */
    co_task_init();

    //    uc_init();

    health_init();

    //    TC1_CH0_CompareInitialize();

    //    app_resettype_status();

    //    thruster_EN_enable_interrupts();

    //    recovery_rcv_init();

    canopen_task_node_start();

    /* Initialize the sequence engine */
    ctrl_sequence_init();

    /*
     * Check lockout status after client error handling and canopen emcy node init
     *   so we can set lockout errors.
     */
    //    client_lockout_status_set(LOCKOUT_ERROR_SET);

//    TraceInfo(TrcMsgAlways,"App Init done",0,0,0,0,0,0);
}

void app_init( void )
{
    app_state = APP_STATE_INIT;
}

void app_task( void *pv)
{
    while(1)
    {
        switch(app_state)
        {
            case APP_STATE_INIT:
                app_state_init();
                app_state = APP_STATE_SERVICE_TASKS;
                break;
            case APP_STATE_SERVICE_TASKS:
                task_flags |= TASK_FLAG_MAIN;
                break;
            default:
                app_state = APP_STATE_INIT;
                break;
        }
    }
}

//size_t APP_error_detail_size_get(void)
//{
//    return sizeof(app_error_detail_t);
//}