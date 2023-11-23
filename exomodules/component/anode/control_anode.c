/**
 * @file    control_anode.c
 *
 * @brief   Implementation for anode controls.
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

#include "control_anode.h"                // This module's header file
#include "anode_mcu.h"                    // Anode MCU shared header file
#include "client-control/client_control.h"// Client services
#include "client-control/client_error_details.h"
#include "ext_decl_define.h"
#include "health/health.h"
#include "hsi_memory.h"
#include "thruster_control.h"
#include "trace/trace.h"// Trace message
#include <stdint.h>
#include "hardwarecontrol_version.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_ANODE_SUBMODULE

typedef enum {
  ANODE_MSG_UNDER_CURRENT_ERROR,
  ANODE_MSG_OVER_CURRENT_ERROR,
  ANODE_MSG_UNDER_VOLTAGE_ERROR,
  ANODE_MSG_OVER_VOLTAGE_ERROR,
  ANODE_MSG_SPARK_TIMEOUT_ERROR,
  ANODE_MSG_BAD_COMMAND_ERROR
} anode_error_table_index_t;

client_bcast_error_stat_t anode_errors[];

client_bcast_error_stat_t anode_errors[] = {
    [ANODE_MSG_UNDER_CURRENT_ERROR] = {.error_code = ERROR_CODE_ANODE_UNDER_CURRENT,
        .error_type = TC_EMCY_REG_CURRENT,
        .fault_handler = FH_ALERT},
    [ANODE_MSG_OVER_CURRENT_ERROR]  = {.error_code = ERROR_CODE_ANODE_OVER_CURRENT,
        .error_type = TC_EMCY_REG_CURRENT,
        .fault_handler = FH_ALERT},
    [ANODE_MSG_UNDER_VOLTAGE_ERROR] = {.error_code = ERROR_CODE_ANODE_UNDER_VOLTAGE,
        .error_type = TC_EMCY_REG_VOLTAGE,
        .fault_handler = FH_ALERT},
    [ANODE_MSG_OVER_VOLTAGE_ERROR]  = {.error_code = ERROR_CODE_ANODE_OVER_VOLTAGE,
        .error_type = TC_EMCY_REG_VOLTAGE,
        .fault_handler = FH_ALERT},
    [ANODE_MSG_SPARK_TIMEOUT_ERROR] = {.error_code = ERROR_CODE_ANODE_NO_SPARK,
        .error_type = CO_EMCY_REG_MANUFACTURER,
        .fault_handler = FH_ALERT},
    [ANODE_MSG_BAD_COMMAND_ERROR]   = {.error_code = ERROR_CODE_ANODE_COMM_UNKNOWN_COMMAND_ERROR,
        .error_type = TC_EMCY_REG_GENERAL,
        .fault_handler = FH_ALERT},
};


// ToDo: this generates a multi-defined error. Why?
//control_anode_t anode_control;

static SemaphoreHandle_t cmd_rsp_semaphore;
static StaticSemaphore_t cmd_rsp_semaphore_buffer;

static SemaphoreHandle_t spark_detect_semaphore;
static StaticSemaphore_t spark_detect_semaphore_buffer;

static void ctrl_anode_error_callback(uint32_t detail);
static void ctrl_anode_plasma_burning_callback(void);
static event_callback_t events = {
    .error          = ctrl_anode_error_callback,
    .running = ctrl_anode_plasma_burning_callback,
};

static version_info_t app_version;

typedef struct {
    float counts_per_volt;
    float counts_per_ampere;
} anode_scaling_t;

static anode_scaling_t anode_scaling = {
    .counts_per_volt = ANODE_COUNTS_PER_VOLT_OUTPUT,
    .counts_per_ampere = ANODE_COUNTS_PER_AMPERE
};



/* Call back for handling command responses */
static int ctrl_anode_rsp_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t rsp_node = {
    .node = {
        .range_low  = RESPONSE_PARAMETERS_ID_ANODE,
        .range_high = RESPONSE_PARAMETERS_ID_ANODE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_anode_rsp_callback,
};

/* Call back function to handle broadcast state changes */
static int ctrl_anode_state_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t bcast_node = {
    .node = {
        .range_low  = BROADCAST_STATE_ID_ANODE,
        .range_high = BROADCAST_STATE_ID_ANODE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_anode_state_bcast_callback,
};

/* Call back function to handle anode variable changes */
static int ctrl_anode_variable_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t variable_node = {
    .node = {
        .range_low  = BROADCAST_VARIABLE_ID_ANODE,
        .range_high = BROADCAST_VARIABLE_ID_ANODE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_anode_variable_bcast_callback,
};

static void ctrl_anode_error_callback(uint32_t detail)
{
    const client_bcast_error_stat_t *perr = NULL;
    uint8_t err = anode_control.status.reason;

    TraceInfo(TrcMsgMcuCtl, "error detected", 0,0,0,0,0,0);
    
    /* The Anode did not spark - notify anyone waiting for a spark */
    xSemaphoreGive(spark_detect_semaphore);

    if(err >= SIZEOF_ARRAY(anode_errors)){
        TraceE2(TrcMsgErr2, "Unknown ANODE error: 0x%02x", err,0,0,0,0,0);
    } else {
        int client_handle_error = client_error_check(ABORT_ANODE_ERROR);
        perr = &anode_errors[err];

        if(client_handle_error || perr->fault_handler == FH_ALERT_LOCKOUT) {
            TraceE2(TrcMsgErr2, "ANODE error: 0x%02x", err,0,0,0,0,0);
            client_control_specific_detail_t a = {.anode_specific_error_detail.anode_status = err,
                                                  .anode_specific_error_detail.anode_adc_val = detail
                                                 };
            ERROR_SET(perr->error_type, perr->error_code, &a);
        }
    }
}

/**
 * Common spark handling.  Called from keeper specializations when they
 * detect a spark
 */
static void ctrl_anode_plasma_burning_callback(void)
{
    xSemaphoreGive(spark_detect_semaphore);
    TraceInfo(TrcMsgMcuCtl, "spark detected", 0,0,0,0,0,0);
    for(uint32_t i = 0; i < SIZEOF_ARRAY(anode_errors); i++){
        ERROR_CLEAR(anode_errors[i].error_type,
                    anode_errors[i].error_code);
    }
}

/**
 * Callback function to handle command responses
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_anode_rsp_callback(message_t *msg)
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
static int ctrl_anode_state_bcast_callback(message_t *msg)
{
    aops.state_bcast(msg->id, msg->data, msg->dlc);
    return 0;
}

/**
 * Callback function to handle variable changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_anode_variable_bcast_callback(message_t *msg)
{
    int err = 0;
    uint8_t *data = msg->data;
    uint16_t id = msg->id;

    variable_type_t *variable_type = (variable_type_t *)data;
    
    if( *variable_type == SCALING_INFO ){
        bcast_scaling_t *bcast_scaling = (bcast_scaling_t *)data;

        TraceInfo(TrcMsgMcuCtl, " id:%x type:SCALING_INFO unit:%d factor:0x%02x%02x%02x%02x",
                  id,bcast_scaling->unit_description,data[7],data[6],data[5],data[4]);
        
        switch( bcast_scaling->unit_description ){
            case COUNTS_PER_VOLT_OUTPUT:
                anode_scaling.counts_per_volt = bcast_scaling->scaling;
                break;
                
            case COUNTS_PER_AMPERE_OUTPUT:
                anode_scaling.counts_per_ampere = bcast_scaling->scaling;
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



int ctrl_anode_isrunning(void)
{
    return aops.is_running();
}

int ctrl_anode_version_get(uint32_t** version)
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

uint16_t ctrl_anode_amperes_to_counts(float amperes){
    return (uint16_t)(amperes * anode_scaling.counts_per_ampere);
};

float ctrl_anode_counts_to_amperes(uint16_t counts){
    return ((float)counts / anode_scaling.counts_per_ampere);
};

uint16_t ctrl_anode_volts_to_counts(float volts){
    return (uint16_t)(volts * anode_scaling.counts_per_volt);
};

float ctrl_anode_counts_to_volts(uint16_t counts){
    return ((float)counts / anode_scaling.counts_per_volt);
};

float ctrl_anode_v_out_get(void)
{
    return (float)hsi_mem.anode_telem.vout/1000;
}

float ctrl_anode_i_out_get(void)
{
#ifdef HALO_SIM
    // We are using Anode power for power decisions, so in the HALO_SIM we need
    // to look up what the current is suppose to be and return that.
    float power = 0;
    float voltage = 0;

    int setpoint = ctrl_sequence_setpoint_get();
    thrust_power_get(setpoint, &power);
    thrust_anode_v_setpoint_get(setpoint, &voltage);

    return (power/voltage);
#else // NOT HALO_SIM
    return  (float)hsi_mem.anode_telem.iout/1000;
#endif // HALO_SIM
}

float ctrl_anode_p_out_get(void)
{
    return ctrl_anode_v_out_get() * ctrl_anode_i_out_get();
}

uint8_t ctrl_anode_ps_state_get()
{
    uint8_t ret;
    ret = anode_control.ps_state;
    return ret;
}

int ctrl_anode_ps_state_set(uint8_t on)
{
    int err;
    if(on == COMMANDED_ON || on == COMMANDED_OFF) {
        anode_control.ps_state = on;
        err = aops.send(CONTROL_ANODE_SET_PS_STATE);
    } else {
        err = -1;
    }
    return err;
}

float_t ctrl_anode_cur_get()
{
    float_t ret;
    ret = anode_control.current;
    return ret;
}

uint16_t ctrl_anode_cur_factored_get()
{
    uint16_t ret;
    ret = anode_control.current_factored;
    return ret;
}

int ctrl_anode_cur_set(float_t current)
{
    int err;
    anode_control.current = current;
    anode_control.current_factored =
        ctrl_anode_amperes_to_counts(anode_control.current);
    err = aops.send(CONTROL_ANODE_SET_CURRENT);
    return err;
}

float_t ctrl_anode_volts_get()
{
    float_t ret;
    ret = anode_control.voltage;
    return ret;
}

uint16_t ctrl_anode_volts_factored_get()
{
    uint16_t ret;
    ret = anode_control.voltage_factored;
    return ret;
}

int ctrl_anode_volts_set(float_t voltage)
{
    int err;
    anode_control.voltage = voltage;
    anode_control.voltage_factored =
        ctrl_anode_volts_to_counts(anode_control.voltage);
    err = aops.send(CONTROL_ANODE_SET_VOLTS);
    return err;
}

int ctrl_anode_monitor_stability(float_t target_current)
{
    int err = __LINE__;
    uint32_t total_count = 60/5;     // we'll check every 5s for 60s max
    uint32_t consecutive = 0;
    float_t measured_i;
    float_t tolerance = target_current * (float_t)0.05;  // assume +/- 5% current range
    float_t target_lo = target_current - tolerance;
    float_t target_hi = target_current + tolerance;

    for(uint32_t i = 0; i < total_count; i++) {
        measured_i = ctrl_anode_i_out_get();

        if((measured_i < target_lo) || (measured_i > target_hi)) {
            // we are outside of tolerance, reset the consecutive counter.
            consecutive = 0;
        }
        else {
            consecutive += 1;
        }

        if(consecutive >= 3) {
            break;
        }

        SEQUENCE_MS_DELAY(5000);
    }

    if(consecutive >= 3) {
        err = 0;
    }

    return err;
}

int ctrl_anode_start(uint32_t timeout)
{
    int err;

    if(timeout == 0) {
        err = ctrl_anode_ps_state_set(COMMANDED_ON);
    }
    else {
        // ToDo: figure this out. Give back the semaphore after success? See also ctrl_keeper_start
        /* Here is how I think this works: */
        /* 1) Take the semaphore with timeout 0. Assuming it is available, this will return immediately
         * and the semaphore is now taken */
        xSemaphoreTake(spark_detect_semaphore, 0);
        /* 2) Now turn on the power supply. If all goes well, a spark will be detected causing
         * ctrl_anode_plasma_burning_callback() to be called where the semaphore will be given back.
         * Presumably, this will not happen before step 3 below. Could it? */
        err = ctrl_anode_ps_state_set(COMMANDED_ON);
        if(!err) {
            /* 3) attempt to take the semaphore again with a timeout. This will block because the
             * semaphore was already taken in step 1. If a spark is detected as a result of step 2,
             * then the semaphore will be given back by ctrl_anode_plasma_burning_callback() allowing
             * the following statement to take the semaphore, returning true. Is that accurate?
             * When is the semaphore given back?
             *
             * If no spark detected, then ctrl_anode_error_callback() is called by hardware control
             * after an internal, hard-coded timer expires. True? If so, that would give the semaphore
             * back.
             *
             * Problem: if the timeout used for the next step is greater than the hard-coded timeout
             * then the following step will be able to take the semaphore after ctrl_anode_error_callback()
             * gives it back, causing this function to return true. Correct? There was no spark but the
             * function returns success? So we have to specify a timeout less than the hard-coded one?
             * ctrl_anode_error_callback() will still be called but this function will have already
             * returned an error.
             *
             * What am I missing????????????    */
            err = xSemaphoreTake(spark_detect_semaphore, (timeout/portTICK_RATE_MS));
            if(err == pdTRUE) {
                err = 0;
            } else {
                err = __LINE__;
                TraceE3(TrcMsgErr3, "Timeout waiting for anode spark",0,0,0,0,0,0);
            }
        }
    }

    return err;
}

int ctrl_anode_reinit(void)
{
    return aops.reinit(NULL);
}

void ctrl_anode_error_handler_init(void)
{
    for(uint32_t i = 0; i < SIZEOF_ARRAY(anode_errors); i++) {
        // Check to make sure we're not trying to register duplicate error codes to fault handler
        int fault_handler_assigned = fh_fault_handlers_registry_check(anode_errors[i].error_code);
        if(!fault_handler_assigned) {
            fh_fault_handlers_register(anode_errors[i].error_code, anode_errors[i].fault_handler);
        }
    }
}

int ctrl_anode_init(void)
{
    int err;

    // Obtain semaphores - anode requires both the command response and spark detect semaphores
    cmd_rsp_semaphore = xSemaphoreCreateBinaryStatic(&cmd_rsp_semaphore_buffer);
    spark_detect_semaphore = xSemaphoreCreateBinaryStatic(&spark_detect_semaphore_buffer);

    // register the component for boot sequence
    client_control_register_component(COMPONENT_ANODE);

    // Initialize the anode operations struct
    err = aops.init(&events, &cmd_rsp_semaphore);

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
