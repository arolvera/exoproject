/**
 * @file    control_keeper.c
 *
 * @brief   Implementation for keeper controls common to both Halo 6 and 12.
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

#include <stdint.h>

#include "client-control/client_control.h"// Client services
#include "client-control/client_error_details.h"
#include "control_keeper.h"// This module's header file
#include "hsi_memory.h"
#include "thruster_control.h"    // Component IDs
#include "trace/trace.h"         // Trace messages
#include "update/update_helper.h"// Sequence Aborts and Abort Codes
#include "hardwarecontrol_version.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define MODULE_NUM      MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM   CLIENT_CONTROL_KEEPER_SUBMODULE



typedef enum {
    KEEPER_MSG_NO_SPARK,
    KEEPER_MSG_CURRENT_LOW,
    KEEPER_MSG_OVER_VOLTAGE_VC_ERROR,
    KEEPER_MSG_OVER_VOLTAGE_IC_ERROR,
    KEEPER_MSG_PARAMETER_ERROR,
    KEEPER_MSG_COMMAND_ERROR,
    KEEPER_MSG_BAD_STATE_ERROR,
} keeper_error_table_index_t;

EXT_DECL client_bcast_error_stat_t keeper_errors[];

client_bcast_error_stat_t keeper_errors[] = {
    [KEEPER_MSG_NO_SPARK] = {.error_code = ERROR_CODE_KEEPER_NO_SPARK_ERROR, .error_type = CO_EMCY_REG_MANUFACTURER, .fault_handler = FH_ALERT},
    [KEEPER_MSG_CURRENT_LOW] = {.error_code = ERROR_CODE_KEEPER_CURRENT_LOW, .error_type = CO_EMCY_REG_CURRENT, .fault_handler = FH_ALERT},
    [KEEPER_MSG_OVER_VOLTAGE_VC_ERROR] = {.error_code = ERROR_CODE_KEEPER_OVER_VOLTAGE_STARTUP, .error_type = CO_EMCY_REG_VOLTAGE, .fault_handler = FH_ALERT},
    [KEEPER_MSG_OVER_VOLTAGE_IC_ERROR] = {.error_code = ERROR_CODE_KEEPER_OVER_VOLTAGE_CONTROL, .error_type = CO_EMCY_REG_VOLTAGE, .fault_handler = FH_ALERT},
    [KEEPER_MSG_PARAMETER_ERROR] = {.error_code = ERROR_CODE_KEEPER_COMM_UNKNOWN_COMMAND_ERROR, .error_type = CO_EMCY_REG_COM, .fault_handler = FH_ALERT},
    [KEEPER_MSG_COMMAND_ERROR] = {.error_code = ERROR_CODE_KEEPER_COMM_UNKNOWN_COMMAND_ERROR, .error_type = CO_EMCY_REG_COM, .fault_handler = FH_ALERT},
    [KEEPER_MSG_BAD_STATE_ERROR] = {.error_code = ERROR_CODE_KEEPER_COMM_UNKNOWN_COMMAND_ERROR, .error_type = CO_EMCY_REG_COM, .fault_handler = FH_ALERT},
};

control_keeper_t keeper_control;

static SemaphoreHandle_t cmd_rsp_semaphore;
static StaticSemaphore_t cmd_rsp_semaphore_buffer;

static SemaphoreHandle_t spark_detect_semaphore;
static StaticSemaphore_t spark_detect_semaphore_buffer;

// Experimentally determined scaling offset for Rev A and earlier hardware
#define KEEPER_V_IN_REV_A_SCALING_OFFSET    0.897

static void ctrl_keeper_error_callback(uint32_t);
static void ctrl_keeper_plasma_burning_callback(void);
static event_callback_t events = {
    .error          = ctrl_keeper_error_callback,
    .running = ctrl_keeper_plasma_burning_callback,
};

static version_info_t app_version;

typedef struct {
    float counts_per_volt_output;
    float counts_per_ampere;
    float counts_per_volt_input;
} keeper_scaling_t;

static keeper_scaling_t keeper_scaling = {
    .counts_per_volt_output = KEEPER_COUNTS_PER_VOLT_FLYBACK,
    .counts_per_ampere = KEEPER_COUNTS_PER_AMPERE,
    .counts_per_volt_input = KEEPER_COUNTS_PER_VOLT_STARTER
};

/* Call back for handling command responses */
static int ctrl_keeper_rsp_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t rsp_node = {
    .node = {
        .range_low  = RESPONSE_PARAMETERS_ID_KEEPER,
        .range_high = RESPONSE_PARAMETERS_ID_KEEPER,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_keeper_rsp_callback,
};


/* Call back function to handle broadcast state changes */
static int ctrl_keeper_state_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t bcast_node = {
    .node = {
        .range_low  = BROADCAST_STATE_ID_KEEPER,
        .range_high = BROADCAST_STATE_ID_KEEPER,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_keeper_state_bcast_callback,
};

/* Call back function to handle keeper variable changes */
static int ctrl_keeper_variable_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t variable_node = {
    .node = {
        .range_low  = BROADCAST_VARIABLE_ID_KEEPER,
        .range_high = BROADCAST_VARIABLE_ID_KEEPER,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_keeper_variable_bcast_callback,
};

static void ctrl_keeper_error_callback(uint32_t detail)
{
    const client_bcast_error_stat_t *perr = NULL;
    uint8_t err = keeper_control.status.reason;
    
    xSemaphoreGive(spark_detect_semaphore);
    
    TraceInfo(TrcMsgMcuCtl, "error detected", 0,0,0,0,0,0);
    
    if(err >= SIZEOF_ARRAY(keeper_errors)) {
        TraceE2(TrcMsgErr2, "Unknown KEEPER error: 0x%02x", err,0,0,0,0,0);
    } else {
        int client_handle_error = client_error_check(ABORT_KEEPER_ERROR);
        perr = &keeper_errors[err];
        
        if(client_handle_error || perr->fault_handler == FH_ALERT_LOCKOUT) {
            TraceE2(TrcMsgErr2, "KEEPER error: 0x%02x", err,0,0,0,0,0);
            client_control_specific_detail_t a = {.keeper_specific_error_detail.keeper_status = err,
                                                  .keeper_specific_error_detail.keeper_adc_val = detail
                                                 };
            ERROR_SET(perr->error_type, perr->error_code, &a);
        }
    }
}
/**
 * Common spark handling.  Called from keeper specializations when they
 * detect a spark
 */
static void ctrl_keeper_plasma_burning_callback(void)
{
    xSemaphoreGive(spark_detect_semaphore);
    TraceInfo(TrcMsgMcuCtl, "spark detected", 0,0,0,0,0,0);
    for(uint32_t i = 0; i < SIZEOF_ARRAY(keeper_errors); i++){
        ERROR_CLEAR(keeper_errors[i].error_type,
                    keeper_errors[i].error_code);
    }
}

/**
 * Callback function to handle command responses
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_keeper_rsp_callback(message_t *msg)
{
    int err;
    uint8_t *data = msg->data;
    uint16_t id = msg->id;

    TraceInfo(TrcMsgMcuCtl, "id:%x d:%04x:%04x:%04x:%04x", id,
            *((uint16_t *)(&data[0])),
            *((uint16_t *)(&data[2])),
            *((uint16_t *)(&data[4])),
            *((uint16_t *)(&data[6])), 0);

    err = xSemaphoreGive(cmd_rsp_semaphore);
    return ~err; // inverting because xSemaphoreGive returns 1 for success and we return 0
}

/**
 * Callback function to handle state changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_keeper_state_bcast_callback(message_t *msg)
{
    int err;
    err = kops.state_bcast(msg->id, msg->data, msg->dlc);
    return err;
}

/**
 * Callback function to handle variable changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_keeper_variable_bcast_callback(message_t *msg)
{
    int err = 0;
    uint8_t *data = msg->data;
    uint16_t id = msg->id;
    
    variable_type_t *variable_type = (variable_type_t *)data;
    
    if ( *variable_type == SCALING_INFO ){
        bcast_scaling_t *bcast_scaling = (bcast_scaling_t *)data;

        TraceInfo(TrcMsgMcuCtl, " id:%x type:SCALING_INFO unit:%d factor:0x%02x%02x%02x%02x",
                  id,bcast_scaling->unit_description,data[7],data[6],data[5],data[4]);

        switch( bcast_scaling->unit_description ){
            case COUNTS_PER_VOLT_OUTPUT:
                keeper_scaling.counts_per_volt_output = bcast_scaling->scaling;
                break;
                
            case COUNTS_PER_AMPERE_OUTPUT:
                keeper_scaling.counts_per_ampere = bcast_scaling->scaling;
                break;
                
            case COUNTS_PER_VOLT_INPUT:
                keeper_scaling.counts_per_volt_input = bcast_scaling->scaling;
                break;
                
            default:
                err = __LINE__;
                break;
        }
    } else if ( *variable_type == VERSION_INFO ){
        mcu_version_t* mcu_version = (mcu_version_t *)data;
        
        app_version.major   = mcu_version->major;
        app_version.minor   = mcu_version->minor;
        app_version.rev     = mcu_version->rev;
        app_version.git_sha = *((uint32_t*)&mcu_version->git_sha);

        TraceInfo(TrcMsgMcuCtl, " id:%x type:VERSION_INFO maj:%02x min:%02x rev:%02x sha:%08x",
                  id,
                  *((uint8_t *)(&app_version.major)),
                  *((uint8_t *)(&app_version.minor)),
                  *((uint8_t *)(&app_version.rev)),
                  *((uint32_t *)(&app_version.git_sha)),
                  0);
    } else {
        err = __LINE__;
    }
    return err;
}

static bool ctrl_keeper_has_error(void)
{
    return kops.has_error();
}

int ctrl_keeper_isrunning(void)
{
    return kops.is_running();
}

int ctrl_keeper_version_get(uint32_t** version)
{
    int err = 0;
    if(version == NULL){
        err = __LINE__;
    } else{
        version[0] = (uint32_t*)&app_version;
        version[1] = (uint32_t*)&app_version.git_sha;
    }
    return err;
}

uint16_t ctrl_keeper_amperes_to_counts(float amperes)
{
    return (uint16_t)(amperes * keeper_scaling.counts_per_ampere);
}

float ctrl_keeper_counts_to_amperes(uint16_t counts)
{
    return ((float)counts / keeper_scaling.counts_per_ampere);
}

uint16_t ctrl_keeper_input_volts_to_counts(float volts)
{
    return (uint16_t)(volts * keeper_scaling.counts_per_volt_input);
}

/* Give this function volts and it will return counts */
uint16_t ctrl_keeper_output_volts_to_counts(float volts)
{
    return (uint16_t)(volts * keeper_scaling.counts_per_volt_output);
}

float ctrl_keeper_input_counts_to_volts(uint16_t counts)
{
    return ((float)counts / keeper_scaling.counts_per_volt_input);
}

float ctrl_keeper_output_counts_to_volts(uint16_t counts)
{
    return ((float)counts / keeper_scaling.counts_per_volt_output);
}

float ctrl_keeper_v_out_get(void)
{
    return (float)hsi_mem.keeper_telem.vout / 1000;
}

float ctrl_keeper_v_in_get(void)
{
    return (float)hsi_mem.keeper_telem.vin / 1000;
}

uint8_t ctrl_keeper_ps_state_get()
{
    uint8_t ret;
    ret = keeper_control.ps_state;
    return ret;
}

int ctrl_keeper_ps_state_set(uint8_t on)
{
    int err;
    if(on == COMMANDED_ON || on == COMMANDED_OFF) {
        keeper_control.ps_state = on;
        err = kops.send(CONTROL_KEEPER_SET_PS_STATE);
    } else {
        err = -1;
    }
    return err;
}

uint16_t ctrl_keeper_state_get()
{
    uint8_t ret;
    ret = keeper_control.status.state;
    return (uint16_t)ret;
}

uint16_t ctrl_keeper_state_stat_get()
{
    uint8_t ret;
    ret = keeper_control.status.reason;
    return (uint16_t)ret;
}

float_t ctrl_keeper_cur_get()
{
    float_t ret;
    ret = keeper_control.current;
    return ret;
}

uint16_t ctrl_keeper_cur_factored_get()
{
    uint16_t ret;
    ret = keeper_control.current_factored;
    return ret;
}

int ctrl_keeper_cur_set(float_t current)
{
    int err;
    keeper_control.current = current;
    keeper_control.current_factored =
        ctrl_keeper_amperes_to_counts(keeper_control.current);
    err = kops.send(CONTROL_KEEPER_SET_CURRENT);
    return err;
}

float_t ctrl_keeper_volts_get()
{
    float_t ret;
    ret = keeper_control.voltage;
    return ret;
}

uint16_t ctrl_keeper_volts_factored_get()
{
    uint16_t ret;
    ret = keeper_control.voltage_factored;
    return ret;
}

int ctrl_keeper_volts_set(float_t voltage)
{
    int err;
    keeper_control.voltage = voltage;
    keeper_control.voltage_factored =
        ctrl_keeper_output_volts_to_counts(keeper_control.voltage);
    err = kops.send(CONTROL_KEEPER_SET_VOLTS);
    return err;
}

int ctrl_keeper_start(uint32_t timeout)
{
    int err;

    if(timeout == 0) {
        err = ctrl_keeper_ps_state_set(COMMANDED_ON);
    }
    else {
        // ToDo: figure this out. See also ctrl_anode_start
        /* Here is how I think this works: */
        /* 1) Take the semaphore with timeout 0. Assuming it is available, this will return immediately
         * and the semaphore is now taken */
        xSemaphoreTake(spark_detect_semaphore, 0);

        /* 2) Now turn on the power supply. If all goes well, a spark will be detected causing
         * ctrl_keeper_plasma_burning_callback() to be called where the semaphore will be given back.
         * Presumably, this will not happen before step 3 below. Could it? */
        err = ctrl_keeper_ps_state_set(COMMANDED_ON);
        if(!err) {
            /* 3) attempt to take the semaphore again with a timeout. This will block because the
             * semaphore was already taken in step 1. If a spark is detected as a result of step 2,
             * then the semaphore will be given back by ctrl_keeper_plasma_burning_callback() allowing
             * the following statement to take the semaphore, returning true. Is that accurate?
             * When is the semaphore given back?
             *
             * If no spark detected, then ctrl_keeper_error_callback() is called by hardware control
             * after an internal, hard-coded timer expires. True? If so, that would give the semaphore
             * back.
             *
             * Problem: if the timeout used for the next step is greater than the hard-coded timeout
             * then the following step will be able to take the semaphore after ctrl_keeper_error_callback()
             * gives it back, causing this function to return true. Correct? There was no spark but the
             * function returns success? So we have to specify a timeout less than the hard-coded one?
             * ctrl_keeper_error_callback() will still be called but this function will have already
             * returned an error.
             *
             * What am I missing????????????    */
            TraceDbg(TrcMsgMcuCtl, "Wait for spark detect", 0,0,0,0,0,0);
            err = xSemaphoreTake(spark_detect_semaphore, (timeout/portTICK_RATE_MS));
            if(err == pdTRUE) {
                err = 0;
            } else {
                err = __LINE__;
                TraceE3(TrcMsgErr3, "Timeout waiting for keeper spark",0,0,0,0,0,0);
            }
        }
    }

    return err;
}

int ctrl_keeper_reinit()
{
    int err;
    /* Belt and suspenders, this 'should' be called at initialization, and this
       already structure defaulted when declared globally above, but just in
       case this gets used as a 're-init' function, default the variables. */
    err = kops.reinit(NULL);
    return err;
}

int ctrl_keeper_wait_err_clear(int ms)
{
    int err = 0;
    bool keeper_has_error = ctrl_keeper_has_error();
    TickType_t xNextWakeTime = xTaskGetTickCount();

    for(int i = 0; i < ms && keeper_has_error; i++) {
        vTaskDelayUntil(&xNextWakeTime, (1/portTICK_PERIOD_MS));
        keeper_has_error = ctrl_keeper_has_error();
    }
    if(keeper_has_error) {
        err = __LINE__;
        TraceE3(TrcMsgErr3, "timeout waiting for keeper error to clear",
                0,0,0,0,0,0);
    }
    return err;
}

void ctrl_keeper_error_handler_init(void)
{
    for(uint32_t i = 0; i < SIZEOF_ARRAY(keeper_errors); i++) {
        // Check to make sure we're not trying to register duplicate error codes to fault handlers
        int fault_handler_assigned = fh_fault_handlers_registry_check(keeper_errors[i].error_code);
        if(!fault_handler_assigned) {
            fh_fault_handlers_register(keeper_errors[i].error_code, keeper_errors[i].fault_handler);
        }
    }
}

int ctrl_keeper_init(void)
{
    int err;

    // Obtain semaphores - keeper requires both the command response and spark detect semaphores
    cmd_rsp_semaphore = xSemaphoreCreateBinaryStatic(&cmd_rsp_semaphore_buffer);
    spark_detect_semaphore = xSemaphoreCreateBinaryStatic(&spark_detect_semaphore_buffer);

    // register the component for boot sequence
    client_control_register_component(COMPONENT_KEEPER);

    // Initialize the keeper operations struct
    err = kops.init(&events, &cmd_rsp_semaphore);

    // register message handler callbacks
    msg_handler_register_callback(&rsp_node);
    msg_handler_register_callback(&bcast_node);
    msg_handler_register_callback(&variable_node);

    // Initialize with 0's. The client will send message after bootup with update.
    app_version.rev = (uint8_t)0;
    app_version.minor = (uint8_t)0;
    app_version.major = (uint8_t)0;
    app_version.reserved = 0;
    app_version.git_sha = (uint32_t)0;

    return err;
}
