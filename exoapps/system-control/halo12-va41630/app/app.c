/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

/* Allocate the memory for all globals defined in thruster_control.h */
#define DECLARE_GLOBALS
#include "thruster_control.h"
#include "hsi_memory.h"
#undef DECLARE_GLOBALS

#define MODULE_NUM MODULE_NUM_APP
#define SUBMODULE_NUM APP_SUBMODULE

#include "app.h"
#include "canopen/co_task.h"
#include "client-control/client_control.h"
#include "client-control/power/client_power.h"
#include "trace/trace.h"
#include "health/health.h"
#include "client-update-server/client_update_server.h"
#include "iacm/iacm.h"
#include "sys/sys_timers.h"
#include "watchdog/hal_watchdog.h"
#include "storage_memory.h"
#include "storage_helper.h"
#include "update/update_command.h"
#include "atomic.h"

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif
typedef enum
{
    /* Application's state machine's initial state. */
    APP_STATE_INIT=0,
    APP_STATE_SERVICE_TASKS,
} APP_STATES;
static APP_STATES app_state;
#define DETAIL_LOG_LENGTH LOG_LENGTH_16


typedef struct app_error_detail{
    base_error_detail_t b_d;
    app_update_status_t app_update_status;
} app_error_detail_t;

static app_error_detail_t error_detail[DETAIL_LOG_LENGTH];



static void* app_error_detail_strategy(void* arg)
{
    if(arg != NULL){
        EH_LOG_ERROR(app_update_status, arg);
    }

    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}

static void app_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, app_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);
    fh_fault_handlers_register(ERROR_CODE_ECP_INSTALL_ERROR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_BACKUP_RESET, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_WATCHDOG_TIMEOUT, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_NRST, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_UNKNOWN_RESET, FH_ALERT);
}

/*
 * Get size of whole error detail struct
 */
size_t app_error_detail_size_get(void)
{
    return sizeof(app_error_detail_t);
}

static volatile int nrst_interrupt_detected = 0;
void rstc_callback(uintptr_t context)
{
    nrst_interrupt_detected = 1;
}

void app_init(void)
{
    app_state = APP_STATE_INIT;
}

static void app_state_init(void)
{
//    RSTC_CallbackRegister(rstc_callback, 0);
    ATOMIC_ENTER_CRITICAL();
    /* If we get to here, we're at least alive enough to clear the crash status */
    sys_stat_crash_set(SYS_CRASH_FALSE);
    /* First thing, make sure all clients are off */
    cp_power_set(false);

    /* First thing first - Figure out the configurations */
//    Hardware_ID = SYSTEM_CONFIG_ID();



    /* Init error handling */
    eh_init();

    /* init app errors */
    app_error_handler_init();

    //Trace init
    trcInit();

    iacm_init();

    //Storage module init
    sm_init();
    sh_init();

//    sys_adc_init_service();
    sys_timer_init();
    /* Init client control error handling */
    client_init();
    /* Init canopen emcy nodes */
    co_task_init();

    uc_init();

    health_init();

//    TC1_CH0_CompareInitialize();

//    app_resettype_status();

//    thruster_EN_enable_interrupts();

//    recovery_rcv_init();

    canopen_task_node_start();

    cus_init();
    ATOMIC_EXIT_CRITICAL();
    /*
     * Check lockout status after client error handling and canopen emcy node init
     *   so we can set lockout errors.
     */
//    client_lockout_status_set(LOCKOUT_ERROR_SET);
    watchdog_enable(30);
    TraceInfo(TrcMsgAlways,"App Init done",0,0,0,0,0,0);
}

/******************************************************************************
  Function:
    void app_task ( void )

  Remarks:
    See prototype in app.h.
 */

void app_task( void *pv)
{
    while(1) {
        watchdog_reset();
        /* Check the application's current state. */
        switch(app_state) {
            /* Application's initial state. */
            case APP_STATE_INIT: {
                app_state_init();
                app_state = APP_STATE_SERVICE_TASKS;
                break;
            }
            case APP_STATE_SERVICE_TASKS: {
                if(nrst_interrupt_detected) {
                    TraceE2(TrcMsgErr2, "*** NRST Interrupt Triggered", 0, 0, 0, 0, 0, 0);
                    nrst_interrupt_detected = 0;
                }
                task_flags |= TASK_FLAG_MAIN;
//                watchdog_reset();

                if(task_flags == TASK_FLAG_ALL) {
                    task_flags = TASK_FLAG_RESET;
                }
//                iacm_integrity_check();
//                sys_adc_service();
                break;
            }
                /* The default state should never be executed. */
            default: {
                /* TODO: Handle error in application's state machine. */
                break;
            }
        }
    }
}

/*******************************************************************************
 End of File
 */

