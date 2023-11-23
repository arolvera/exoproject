/**
 * @file    client_control.c
 *
 * @brief   Implementation for client control functions.
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

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_SUBMODULE

#include "bit/control_bit.h"
#include "client-control/power/client_power.h"
#include "client_booted.h"
#include "client_control.h"
#include "client_error_details.h"
#include "client_health.h"
#include "client_p.h"
#include "client_service.h"
#include "cmd/command_anode.h"
#include "cmd/command_keeper.h"
#include "cmd/command_magnets.h"
#include "control_anode.h"
#include "error_codes.h"
#include "health/health.h"
#include "keeper/control_keeper.h"
#include "magnet/control_magnets.h"
#include "mcu_include.h"
#include "setpoint/control_setpoint.h"
#include "storage/storage_helper.h"
#include "throttle/control_throttle.h"
#include "thruster-start/control_thruster_start.h"
#include "trace/trace.h"
#include "valve/control_valves.h"
#include "user-setting-values/client_control_usv.h"

typedef struct client_control_error_detail {
  base_error_detail_t b_d;
  client_control_specific_detail_t client_control_specific_detail;
} client_control_error_detail_t;

/* do not use this one, use the pointer, so you know if it is initialized */
static OSAL_MUTEX_HANDLE_TYPE mutex_client_power;

static exec_app_versions_t device_version_info[DEVICE_MAXIMUM];

#define DETAIL_LOG_LENGTH LOG_LENGTH_16

static client_control_error_detail_t error_detail[DETAIL_LOG_LENGTH];

/* Put into factory design pattern */
static void *client_control_error_detail_strategy(void *arg);

#define CONSECUTIVE_MCI_HSI_MISSES_ALLOWED  4

/* Can Open Queue Item size */
#define QUEUE_ITEM_SIZE_SERVICE (sizeof(client_service_t))
#define BUFFER_COUNT_SERVICE 20
static QueueHandle_t xServiceQueue;
static OSAL_STATIC_QUEUE_TYPE xServiceStaticQueue;
static uint8_t ucServiceQueueStorageArea[QUEUE_ITEM_SIZE_SERVICE * BUFFER_COUNT_SERVICE];

/* How long to wait for a function to process before calling the service queue */
#define CLIENT_SERVICE_DELAY (10 / portTICK_PERIOD_MS)
#define client_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define client_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 3)
static TaskHandle_t client_task_handle;
static StackType_t client_task_stack[client_TASK_STACK_SIZE];
static StaticTask_t client_TaskBuffer;

/** * Check client health before going to Standby
 */
static bool client_intial_health_checked = false;
/* Tracks power on/off settings.  Should be set in exactly ONE place, right
 * where the client power off command is called.  Presently, its only used
 * to block sending commands and causing a backup when other tasks are trying
 * to talk to MCUs
 */
static bool master_client_power_state = false;

/**
 * Store state of conditioning sequences
 */
static volatile bool condition_task_running = false;

/**
 * Translate common command message into human readable form
 * @param msg
 */
static void client_cmd_trace(client_msg_t *msg)
{
    static const char *on_off = "ON_OFF";
    static const char *set_voltage = "v";
    static const char *set_current = "i";
    static const char *set_flow = "flow";

    static const char *no_specifier = "none";
    static const char *magnet = "magnet";
    static const char *cat_high_flow = "hf";
    static const char *cat_low_flow = "lf";
    static const char *anode_flow = "anode";

    static const char *unknown = "unk";

    uint8_t c = msg->data[0];
    const char *cmd = NULL;

    uint8_t s = msg->data[1];
    const char *spec = NULL;

    uint16_t value = *((uint16_t *)(&msg->data[2]));

    switch(c) {
        case ON_OFF :
            cmd = on_off;
            break;
        case SET_VOLTAGE :
            cmd = set_voltage;
            break;
        case SET_CURRENT :
            cmd = set_current;
            break;
        case SET_FLOW :
            cmd = set_flow;
            break;
        default:
            cmd = unknown;
    }

    switch(s) {
        case NO_SPECIFIER :
            spec = no_specifier;
            break;
        case MAGNET :
            spec = magnet;
            break;
        case CAT_HIGH_FLOW :
            spec = cat_high_flow;
            break;
        case CAT_LOW_FLOW :
            spec = cat_low_flow;
            break;
        case ANODE_FLOW :
            spec = anode_flow;
            break;
        default:
            spec = unknown;
    }
    TraceInfo(TrcMsgMcuCtl, "id:%x:cmd:%s:spec:%s:val:%d",
              msg->cobid, (int)cmd, (int)spec, value, 0, 0);
}

/**
 * Get state of conditioning sequence
 * @return
 */
bool client_get_conditioning_task_state(void)
{
    return condition_task_running;
}

/**
 * Sets state of conditioning sequence
 * @return
 * @param is_conditioning
 */
void client_set_conditioning_task_state(bool is_conditioning)
{
   condition_task_running = is_conditioning;
}

/**
 * Send a message to the client and wait for a response.  To wait for a response
 * this function will take the semaphore in the provided structure.  The caller
 * module should post the semaphore when a response is received.
 *
 * If the caller does not wish to wait for a response, the semaphore timeout
 * should be set to zero.
 *
 * @param msg structure containing message, semaphore, and response timeout
 *
 * @return 0 on success, non-zero otherwise
 */
int client_cmd_send(client_msg_t *msg)
{
    int err;

    if(master_client_power_state == false) {
        err = __LINE__;
        TraceInfo(TrcMsgMcuCtl, "clients powered off, send blocked", 0, 0, 0, 0, 0, 0);
    }else {
        TraceInfo(TrcMsgMcuCtl, "id:%x d:%04x:%04x:%04x:%04x", msg->cobid,
                  *((uint16_t *)(&msg->data[2])),
                  *((uint16_t *)(&msg->data[0])),
                  *((uint16_t *)(&msg->data[6])),
                  *((uint16_t *)(&msg->data[4])),0);


        err = send_msg(msg->cobid, msg->data, msg->dlc, 0);
        if(!err) {
            client_cmd_trace(msg);
            err = xSemaphoreTake(*(msg->psem), msg->timeout);
            if(err != pdTRUE) {
                TraceE3(TrcMsgMcuCtl, "Command timeout id:0x%x to:%d",
                        msg->cobid, msg->timeout, 0, 0, 0, 0);
                err = __LINE__;
            }else {
                err = 0;
            }
        }
    }
    return err;
}

/*
 * @brief Read redundant execution headers to get stored version and git sha information
 * @return 0 on success, !0 otherwise
 */
static int client_exec_versions_read(const device_type_t device)
{
    int err;

    /* if valid component requested */
    if(device >= DEVICE_MAXIMUM) {
        err = __LINE__;
    }else {
        uint32_t version = 0;
        uint32_t git_sha = 0;

        err = sh_get_active_versions(device, &version, &git_sha);

        if(!err) {
            err = 0;
            device_version_info[device].version = version;
            device_version_info[device].git_sha = git_sha;
        }else {
            err = __LINE__;
        }
    }
    return err;
}

void client_internal_version_OD_ptrs_populate(uint32_t *(*firmware_versions)[2])
{
    for(int i = DEVICE_ECPK; i < DEVICE_MAXIMUM; i++) {
        firmware_versions[i][0] = &device_version_info[i].version;
        firmware_versions[i][1] = &device_version_info[i].git_sha;
    }
}

/**
 * Clear any steady state errors that need to be start off as not set
 * when going to Steady State.
 */
void client_steady_state_err_clr(void)
{
    ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_HIGH_FAULT);
    ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_HIGH_WARN);
    ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_LOW_WARN);
    ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_THROTTLING_FAULT);
}

/**
 * Function to reset the system
 *
 * @param power on or power off option
 * @return 0
 */
void client_power_state_set(bool power_on)
{
    OSAL_MUTEX_Lock(&mutex_client_power, OSAL_WAIT_FOREVER);
    /* master_client_power_state tracks the power state of the clients. At the
     * moment it is only used to block commands to the client when it is powered
     * down.
     * We do not want master_client_power_state to be changed again until we
     * have aborted running sequences and started the power sequence.
     */
    master_client_power_state = power_on;

    /* Reset so it can be checked again on then next transition */
    client_intial_health_checked = false;

    /* Do not pull the rug from under the sequences */
    ctrl_sequence_abort();

    if(power_on) {
        TraceDbg(TrcMsgAlways,"Power on clients",0,0,0,0,0,0);
        client_power_up_timer();    // This sets thruster state to transition to standby
        //        bootloader_client_init();
        cp_power_set(power_on);
    }else {
        TraceDbg(TrcMsgAlways,"Power down clients",0,0,0,0,0,0);
        client_power_down_timer();
        cp_power_set(power_on);
        //        bootloader_client_init();
    }
    OSAL_MUTEX_Unlock(&mutex_client_power);
}

void client_reset(void)
{
    OSAL_MUTEX_Lock(&mutex_client_power, OSAL_WAIT_FOREVER);
    /* master_client_power_state tracks the power state of the clients. At the
     * moment it is only used to block commands to the client when it is powered
     * down.
     * We do not want master_client_power_state to be changed again we until we
     * have aborted running sequences and started the reset sequence.
     */
    master_client_power_state = false;

    /* This will reset to True after the clients reset */
    client_intial_health_checked = false;

    /* Do not pull the rug from under the sequences */
    ctrl_sequence_abort();

    /* We'll use the power up timer for resets. This sets thruster state to
     * TCS_TRANISTION_STANDBY */
    client_power_up_timer();

    cp_reset_all();

    OSAL_MUTEX_Unlock(&mutex_client_power);
}

void client_reset_complete(void)
{
    OSAL_MUTEX_Lock(&mutex_client_power, OSAL_WAIT_FOREVER);
    master_client_power_state = true;
    OSAL_MUTEX_Unlock(&mutex_client_power);
}

void client_update_state()
{
    /* Temporary Thruster State - don't yank around the real one */
    uint32_t tcs = TCS_CO_INVALID;

    /* In addition to the raw state, check if it is 'running', that keeps
     * the "is running" logic in the keeper control module */
    int keeper_is_running = ctrl_keeper_isrunning();
    int anode_is_running = ctrl_anode_isrunning();

    /* First thing first - If they are powered off we are done */
    if(client_boot_status_get_state() == CBS_POWERED_OFF || Thruster_NMT_State != TCS_CO_OPERATIONAL) {
        tcs = TCS_POWER_OFF;
    }

    if(tcs == TCS_CO_INVALID) {
        uint32_t bit = ctrl_bit_isrunning();
        if(bit) {
            tcs = TCS_BIT_TEST;
        }
    }

    if(tcs == TCS_CO_INVALID) {
        if(condition_task_running) {
            tcs = TCS_CONDITIONING;
        }
    }

    /* Check if a sequence is running */
    if(tcs == TCS_CO_INVALID) {
        tcs = ctrl_ts_in_transition();
    }

    /* A startup sequence is not running, so check the MCU states */
    if(tcs == TCS_CO_INVALID) {
        if(anode_is_running) {
            tcs = TCS_STEADY_STATE;
            if(keeper_is_running) {
                tcs |= TCS_READY_MODE << 4;
            }
        }else if(keeper_is_running) {
            // Halo12 will never have Keeper running without anode so this
            // state should never be observed.
            tcs = TCS_READY_MODE;
        }
    }

    /* The clients are powered on and nothing is running. See if they are booted
     * or booting  */
    if(tcs == TCS_CO_INVALID && Thruster_NMT_State == TCS_CO_OPERATIONAL) {
        if(client_boot_status_get_state() == CBS_BOOTING) {
            tcs = TCS_TRANISTION_STANDBY;
        }else if(client_boot_status_get_state() == CBS_BOOTED) {
            /* Once the clients are booted wait for HSI to be valid and
             *  then run the voltage compensation check - else we are still
             *  transitioning to standby */
            if(!client_intial_health_checked && health_valid()) {
                client_health_bootup_check();
                client_intial_health_checked = true;
            }
            if(client_intial_health_checked) {
                tcs = TCS_STANDBY;
            }else {
                tcs = TCS_TRANISTION_STANDBY;
            }
        }
    }
    if(Thruster_NMT_State >= TCS_CO_PREOP) {
        /* Putting into the lockout state before it makes it to standby
         * causes chaos and confusion with the NMT boot states.  Let it go to
         * operation and initialize and then got to standby before locking it
         * out */
        //        int lo = client_lockout_val_get();
        //        if(lo == IACM_THRUSTER_STATE_LOCKOUT) {
        //            if(client_lockout_timer_get()) {
        //                client_lockout_update_timer();
        //                if(tcs > TCS_TRANISTION_STANDBY) {
        //                    tcs = TCS_LOCKOUT;
        //                }
        //            }else {
        //                /* Timer expired */
        //                client_lockout_expired();
        //            }
        //        }
    }
    Thruster_State = tcs; /* Update in ONE place */
}

/*
 * Clear errors if we successfully transition through steps needed to fire thruster
 */
static void client_mode_err_clear(void)
{
    int tcs_stat = (int)ctrl_ts_ready_mode_stat();
    if(SEQUENCE_STATUS_GET(tcs_stat) == SEQ_STAT_SUCCESS) {
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_READY_MODE_FAULT);
    }

    tcs_stat = (int)ctrl_ts_steady_state_stat();
    if(SEQUENCE_STATUS_GET(tcs_stat) == SEQ_STAT_SUCCESS) {
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_STEADY_STATE_FAULT);
    }
}

static int client_safety_keeper_check(const uint32_t tcs)
{
    int err = 0;
    client_control_specific_detail_t d = { .keeper_ov.keeper_voltage = 0, .keeper_ov.lf_pressure = 0 };
    uint32_t keeper_is_running  = ctrl_keeper_isrunning();

    if(keeper_is_running && tcs != TCS_TRANSITION_READY_MODE && tcs != TCS_CONDITIONING) {
        keeper_septic_limits_t kv_limits = cc_usv_limit_keeper_overvoltage_get();
        int keeper_voltage = (int)ctrl_keeper_v_out_get();

        d.keeper_ov.keeper_voltage = keeper_voltage;
        d.keeper_ov.lf_pressure = ctrl_valves_cath_pressure();

        if (keeper_voltage > kv_limits.critical)  {
            err = __LINE__;
            int fault_set = eh_fault_status_get(ERROR_CODE_KEEPER_OVER_VOLTAGE);
            ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_KEEPER_OVER_VOLTAGE, &d);
            if(!fault_set) {
                TraceE2(TrcMsgErr2, "Keeper voltage too high t:%d v:%d p:%d",
                        kv_limits.critical, keeper_voltage, d.keeper_ov.lf_pressure, 0,0,0);
            }
        } else if(keeper_voltage > kv_limits.warn) {
            err = __LINE__;
            int fault_set = eh_fault_status_get(ERROR_CODE_KEEPER_WARN_VOLTAGE);
            ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_KEEPER_WARN_VOLTAGE, &d);
            if(!fault_set) {
                TraceE3(TrcMsgErr3, "Keeper voltage waring t:%d v:%d p:%d",
                        kv_limits.warn, keeper_voltage, d.keeper_ov.lf_pressure, 0,0,0);
            }

        } else if(keeper_voltage < kv_limits.clear) {
            ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_KEEPER_OVER_VOLTAGE);
            ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_KEEPER_WARN_VOLTAGE);
        }
    }
    return err;
}

int client_safety_checks(const uint32_t tcs)
{
    int err = 0;

    if(Thruster_NMT_State == TCS_CO_OPERATIONAL && health_valid()) {
        client_vin_check();

        /* Ignore error return here, anything that needs to be handled can
         * be handled by the fault handler.  We want to do the rest of the
         * checks here for the case where the keeper is on in steady state.
         * The error handler can turn the keeper off, but leave it in steady
         * state.  Hence, we need to do the other error checking below.
         */
        client_safety_keeper_check(tcs);

        uint32_t throttling = ctrl_throttle_is_throttling();
        if(THRUSTER_IN_STEADY_STATE() && !throttling) {
            cmd_magnet_current_check();
            client_setpoint_power_check();
        }
        /* If its not transitioning to steady state or in steady state the anode
         * flow and magnets should be off.
         */
        if(tcs < TCS_TRANSITION_STEADY_STATE) {
            if(ctrl_valves_anode_flow_factored_get()) {
                ctrl_valves_anode_flow_set(0);
            }
            uint32_t magnet_state  = cmd_magnet_state_get();
            if(magnet_state != OFF_SET_POINT) {
                TraceDbg(TrcMsgDbg, "mag on err. tcs:%x", tcs, 0,0,0,0,0);
                cmd_magnet_state_set(OFF_SET_POINT);
            }
        }
        /* If its not transitioning to ready mode or in ready mode the low and high
         * flow and magnets should be off.
         */
        if(tcs < TCS_TRANSITION_READY_MODE) {
            if(ctrl_valves_cathode_lf_get()) {
                ctrl_valves_cathode_lf_set(0);
            }
            if(ctrl_valves_cathode_hf_get()) {
                ctrl_valves_cathode_hf_set(0);
            }
        }
        /* If the thruster is in standby or greater, HSI should be going.  IF
         *  there are too many consecutive misses from the MCUs take action  */
        if(tcs > TCS_TRANISTION_STANDBY) {
            uint32_t cm = health_device_misses_get(); /* Consecutive Misses */
            if(cm > CONSECUTIVE_MCI_HSI_MISSES_ALLOWED) {
                client_control_specific_detail_t d = { 0 };
                d.boot_status = (1 << BS_HSI_MISSES_EXCEEDED);
                ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_MCU_THRUSTER_FAULT, &d);
            } else {
                ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_MCU_THRUSTER_FAULT);
            }
        }
    }
    return err;
}

/**
 * Service the client task queue.  De-queue's the next task in the queue and
 * executes it.
 */
static void client_service(void)
{
    client_service_t service;
    BaseType_t xStatus = xQueueReceive(xServiceQueue, &service, CLIENT_SERVICE_DELAY);
    if(xStatus == true) {
        // Service needed
        service.cb(service.params);
    }
    client_boot_timout_check();
    client_update_state();
    client_safety_checks(Thruster_Control_State);
    client_mode_err_clear();
}

/*
 * Check the client's reported error to see who should handle it
 * 
 * Clients always handle errors unless sequence is running, and does NOT cause an abort
 * in which case we are purposely ignoring that clients errors (started anode, keepers etc...)
 * 
 * @param abort: Is the client telling us to abort?
 * @param error_code: The clients thrown error code
 * 
 * @return device_action: Tell the client whether to handle its own error
 * (1 => client handles own error, 0 => sequence engine handles error)
 */
int client_error_check(sequence_abort_error_t abort_err)
{
    int client_handle_error = 1;

    uint32_t seq_status = ctrl_sequence_status_get();
    if(SEQUENCE_IS_RUNNING(seq_status)) {
        int abort = ctrl_sequence_abort_error(abort_err);
        if(!abort) {
            client_handle_error = 0;
        }
    }

    return client_handle_error;
}

/**
 * This WILL power off the clients.
 */
inline void client_emcy_shutdown(void)
{
    /* This will cause the system to changed NMT States and power off the 
     * MCUs, which will callback all the right things in this and the power
     * control module.     */
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_MCU_THRUSTER_FAULT, NULL);
}

size_t client_error_detail_size_get(void)
{
    return sizeof(client_control_error_detail_t);
}

static void *client_control_error_detail_strategy(void *arg)
{
    if(arg != NULL) {
        EH_LOG_ERROR(client_control_specific_detail, arg);
    }

    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}

static void ctrl_client_task(void *pvParameters)
{
    while(1) {
        client_service();
    }
}

/**
 * init and register error codes for error handling
 * 
 * @param void
 * @return void
 */
void client_control_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, client_control_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);

#if __DEBUG
    fh_fault_handlers_register(ERROR_CODE_MCU_BOOT_ERR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_MCU_THRUSTER_FAULT, FH_ALERT);
#endif
    fh_fault_handlers_register(ERROR_CODE_THRUSTER_POWER_HIGH_WARN, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_THRUSTER_POWER_LOW_WARN, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_THRUSTER_POWER_HIGH_FAULT, FH_ALERT_THRUSTER_SHUTDOWN);
    fh_fault_handlers_register(ERROR_CODE_HKM_UNDER_VOLTAGE, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_HKM_OVER_VOLTAGE, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_KEEPER_OVER_VOLTAGE, FH_ALERT_THRUSTER_SHUTDOWN);
    fh_fault_handlers_register(ERROR_CODE_KEEPER_WARN_VOLTAGE, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_SQNC_CONDITIONING_FAULT, FH_ALERT);
    /*
     * init individual client error handlers
     */
    sq_control_error_handler_init();
    ctrl_anode_error_handler_init();
    ctrl_magnet_error_handler_init();
    ctrl_valves_error_handler_init();
    ctrl_keeper_error_handler_init();
}

static void client_start_task(void)
{
    client_task_handle = xTaskCreateStatic(ctrl_client_task, "Client Task", client_TASK_STACK_SIZE,
                                           NULL, client_TASK_PRIORITY, client_task_stack, &client_TaskBuffer);
}

/**
 * Initialize all client MCUs. 
 */
void client_init()
{
    static StaticSemaphore_t xMutexBuffer;
    OSAL_MUTEX_Create(&mutex_client_power, &xMutexBuffer, "cli_pwr");

    /**
     * The sequence engine must be initialized before the keeper, otherwise
     * we end up id default_handler hell. Still not sure of the reason why
     * but I want to move on. Alternative is to power down the keeper at the
     * beginning of app_state_init() but that feels like a kludge.
     */
    ctrl_sequence_init();
    cmd_keeper_init();
    cmd_anode_init();
    cmd_magnet_init();
    ctrl_valves_init();

    // ToDo: cus_init();

    client_control_error_handler_init();

    int err = 0;
    /*
     * Read device info from execution header
     */
    for(int i = DEVICE_ECPK; i < DEVICE_MAXIMUM && !err; i++) {
        err = client_exec_versions_read(i);
    }

    OSAL_QUEUE_CreateStatic(&xServiceQueue, BUFFER_COUNT_SERVICE, QUEUE_ITEM_SIZE_SERVICE,
                                       ucServiceQueueStorageArea, &xServiceStaticQueue, "srvc");
    client_service_init(&xServiceQueue);
//    client_lockout_init();

    client_start_task();
}
