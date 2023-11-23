/**
 * @file    control_magnets.c
 *
 * @brief   Implementation for magnet controls.
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

#include <string.h>
#include <stdint.h>

#include "client-control/client_control.h"// Client services
#include "user-setting-values/client_control_usv.h"
#include "control_magnets.h" // This module's header file
#include "health/health.h"   // For health resourse structure
#include "magnet_mcu.h"
#include "thruster_control.h"// Component Info
#include "client-control/client_error_details.h"
#include "mcu_include.h"
#include "trace/trace.h"
#include "hardwarecontrol_version.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_MAGNETS_SUBMODULE

/**
 * @brief Magnet errors itemizes each error type that can be registered for the magnets.
 * Not all errors are applicable to each of the thrusters given magnet and MCU configurations.
 */
typedef enum {
  MAGNET_MSG_INNER_UNDER_CURRENT_ERROR,
  MAGNET_MSG_INNER_OVER_CURRENT_ERROR,
  MAGNET_MSG_OUTER_UNDER_CURRENT_ERROR,
  MAGNET_MSG_OUTER_OVER_CURRENT_ERROR,
  MAGNET_MSG_INNER_VOLTAGE_ERROR,
  MAGNET_MSG_OUTER_VOLTAGE_ERROR,
  MAGNET_MSG_INNER_PWM_RAILED_ERROR,
  MAGNET_MSG_OUTER_PWM_RAILED_ERROR,
  MAGNET_MSG_TEMPERATURE_ERROR,
  MAGNET_MSG_BAD_COMMAND_ERROR,
} magnet_error_table_index_t;

client_bcast_error_stat_t magnet_errors[] = {
    [MAGNET_MSG_INNER_UNDER_CURRENT_ERROR] = {.error_code = ERROR_CODE_MAGI_MAGNET_UNDER_CURRENT,
                                              .error_type = TC_EMCY_REG_CURRENT,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_INNER_OVER_CURRENT_ERROR]  = {.error_code = ERROR_CODE_MAGI_MAGNET_OVER_CURRENT,
                                              .error_type = TC_EMCY_REG_CURRENT,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_OUTER_UNDER_CURRENT_ERROR] = {.error_code = ERROR_CODE_MAGO_MAGNET_UNDER_CURRENT,
                                              .error_type = TC_EMCY_REG_CURRENT,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_OUTER_OVER_CURRENT_ERROR]  = {.error_code = ERROR_CODE_MAGO_MAGNET_OVER_CURRENT,
                                              .error_type = TC_EMCY_REG_CURRENT,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_INNER_VOLTAGE_ERROR]       = {.error_code = ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,
                                              .error_type = TC_EMCY_REG_VOLTAGE,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_OUTER_VOLTAGE_ERROR]       = {.error_code = ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,
                                              .error_type = TC_EMCY_REG_VOLTAGE,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_INNER_PWM_RAILED_ERROR]    = {.error_code = ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,
                                              .error_type = TC_EMCY_REG_VOLTAGE,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_OUTER_PWM_RAILED_ERROR]    = {.error_code = ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,
                                              .error_type = TC_EMCY_REG_VOLTAGE,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_TEMPERATURE_ERROR]         = {.error_code = ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,
                                              .error_type = TC_EMCY_REG_VOLTAGE,
                                              .fault_handler = FH_ALERT_THRUSTER_SHUTDOWN},
    [MAGNET_MSG_BAD_COMMAND_ERROR]         = {.error_code = ERROR_CODE_MAGO_COMM_UNKNOWN_COMMAND_ERROR,
                                              .error_type = CO_EMCY_REG_COM,
                                              .fault_handler = FH_ALERT},
};

static SemaphoreHandle_t cmd_rsp_semaphore;
static StaticSemaphore_t cmd_rsp_semaphore_buffer;

static void ctrl_magnet_error_callback(uint32_t detail);
static void ctrl_magnet_running_callback(void);
static event_callback_t events = {
    .error   = ctrl_magnet_error_callback,
    .running = ctrl_magnet_running_callback
};

static version_info_t app_version;

typedef struct {
    float counts_per_volt;
    float counts_per_ampere;
} magnet_scaling_t;

static magnet_scaling_t magnet_scaling = {
    .counts_per_volt = MAGNET_COUNTS_PER_VOLT,
    .counts_per_ampere = MAGNET_COUNTS_PER_AMPERE
};



uint16_t ctrl_magnet_volts_to_counts(float volts){
    return (uint16_t)(volts * magnet_scaling.counts_per_volt);
}

float ctrl_magnet_counts_to_volts(uint16_t counts){
    return ((float)counts / magnet_scaling.counts_per_volt);
}

uint16_t ctrl_magnet_amperes_to_counts(float amperes){
    return (uint16_t)(amperes * magnet_scaling.counts_per_ampere);
}

float ctrl_magnet_counts_to_amperes(uint16_t counts){
    return ((float)counts / magnet_scaling.counts_per_ampere);
}

/*  LOWER & UPPER IDs are for registering CAN message callback range
 * 
 *  CANOPEN IDs are for processing messages
 * 
 *  Magnets are different in that there are two that are handled in this control
 *  module.  To remain flexible, dynamically calculate the lower and upper msg
 * ids, just in case they git flipped the ranges will not be messed up.
 */
/*
#define MAGNET_ID_LOWER (CAN_OPEN_NODE_ID_MAGNET_OUTER < CAN_OPEN_NODE_ID_MAGNET_INNER ? \
                         CAN_OPEN_NODE_ID_MAGNET_OUTER : CAN_OPEN_NODE_ID_MAGNET_INNER )

#define MAGNET_ID_UPPER (CAN_OPEN_NODE_ID_MAGNET_OUTER > CAN_OPEN_NODE_ID_MAGNET_INNER ? \
                         CAN_OPEN_NODE_ID_MAGNET_OUTER : CAN_OPEN_NODE_ID_MAGNET_INNER )
*/



/* Call back for handling command responses */
static int ctl_magnet_rsp_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t rsp_node = {
    .node = {
        .range_low  = RESPONSE_PARAMETERS_ID_BASE | COMM_ID_MAGNET,
        .range_high = RESPONSE_PARAMETERS_ID_BASE | COMM_ID_MAGNET,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctl_magnet_rsp_callback,
};

/**
 * @brief Callback function to handle command responses
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return 0 = success, non-zero otherwise
 */
static int ctl_magnet_rsp_callback(message_t *msg)
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

/* Call back function to handle broadcast state changes */
static int ctrl_magnet_state_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t bcast_node = {
    .node = {
        .range_low  = BROADCAST_STATE_ID_MAGNET,
        .range_high = BROADCAST_STATE_ID_MAGNET,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_magnet_state_bcast_callback,
};

/**
 * @brief Call back function to handle magnet variable changes
 * @param msg Data structure containing info about the changed variable
 * @return 0 = success, non-zero otherwise
 */
static int ctrl_magnet_variable_bcast_callback(message_t *msg);

/**
 * @brief Call back pointer & message IDs to register for client messages
 */
static msg_callback_t variable_node = {
    .node = {
        .range_low  = BROADCAST_VARIABLE_ID_BASE | COMM_ID_MAGNET,
        .range_high = BROADCAST_VARIABLE_ID_BASE | COMM_ID_MAGNET,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_magnet_variable_bcast_callback,
};

static void ctrl_magnet_error_callback(uint32_t detail)
{
    const client_bcast_error_stat_t *perr = NULL;
    uint8_t err = magnet_control.status.reason;

    TraceInfo(TrcMsgMcuCtl, "error detected", 0,0,0,0,0,0);
    
    if(err >= SIZEOF_ARRAY(magnet_errors)) {
        TraceE2(TrcMsgErr2, "Unknown MAGNET error: 0x%02x", err,0,0,0,0,0);
    } else {
        int client_handle_error = client_error_check(ABORT_MAGNET_ERROR);
        perr = &magnet_errors[err];

        if(client_handle_error || perr->fault_handler == FH_ALERT_LOCKOUT) {
            TraceE2(TrcMsgErr2, "Magnet error: 0x%02x", err,0,0,0,0,0);
            client_control_specific_detail_t a = { .magnet_specific_error_detail.magnet_status = err,
                                                   .magnet_specific_error_detail.magnet_adc_val = detail
                                                 };
            ERROR_SET(perr->error_type, perr->error_code, &a);
        }
    }
}

static void ctrl_magnet_running_callback(void)
{
    for(uint i = 0; i < SIZEOF_ARRAY(magnet_errors); i++){
        ERROR_CLEAR(magnet_errors[i].error_type,
                    magnet_errors[i].error_code);
    }
}

/**
 * Callback function to handle state changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_magnet_state_bcast_callback(message_t *msg)
{
    mops.state_bcast(msg->id, msg->data, msg->dlc);
    return 0;
}

/**
 * Callback function to handle variable changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_magnet_variable_bcast_callback(message_t *msg)
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
                magnet_scaling.counts_per_volt = bcast_scaling->scaling;
                break;
                
            case COUNTS_PER_AMPERE_OUTPUT:
                magnet_scaling.counts_per_ampere = bcast_scaling->scaling;
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

uint8_t ctrl_magnet_ps_state_get(void)
{
    return magnet_control.ps_state;
}

int ctrl_magnet_ps_state_set(uint8_t state)
{
    int err;
    if(state == COMMANDED_ON || state == COMMANDED_OFF) {
        magnet_control.ps_state = state;
        err = mops.send(CONTROL_MAGNET_SET_PS_STATE);
    } else {
        err = __LINE__;
    }
    return err;
}

int ctrl_magnets_version_get(uint32_t** version)
{
    int err = 0;

    if(version == NULL){
        err = __LINE__;
    } else {
        version[0] = (uint32_t*)&app_version;
        version[1] = (uint32_t*)&app_version.git_sha;
    }
    return err;
}

float ctrl_magnet_current_get(void)
{
    float ret;
    ret = magnet_control.current;
    return ret;
}

uint16_t ctrl_magnet_current_factored_get(void)
{
    uint16_t ret;
    ret = magnet_control.current_factored;
    return ret;
}

int ctrl_magnet_current_set(float current)
{
    int err;
    magnet_control.current = current;
    magnet_control.current_factored =
        ctrl_magnet_amperes_to_counts(magnet_control.current);
    err = mops.send(CONTROL_MAGNET_SET_CURRENT);
    return err;
}

int ctrl_magnet_current_check(void)
{
    int err = 0;

    uint16_t threshold = magnet_control.current_factored;
    uint16_t current_err = cc_usv_limit_magnet_error_get();
    uint16_t high_threshold = threshold + current_err;
    uint16_t low_threshold = threshold - current_err;

    uint16_t iout = ctrl_magnet_halo12_i_counts();

    if(mops.is_running()) {
        if(iout < low_threshold){
            err = ERROR_CODE_MAGO_MAGNET_UNDER_CURRENT;
        } else if(iout > high_threshold){
            err = ERROR_CODE_MAGO_MAGNET_OVER_CURRENT;
        }
        if(err){
            TraceE2(TrcMsgAlways, "omag i err  i:%d t:%d e:%d i:%d s:%d",
                    iout,
                    (int)(ctrl_magnet_counts_to_amperes(threshold) * 1000),
                    (int)(ctrl_magnet_counts_to_amperes(current_err) * 1000),
                    (int)(magnet_control.current * 1000), magnet_control.ps_state, 0);
            magnet_specific_error_detail_t m_d = {0};
            m_d.magnet_status = magnet_control.status.reason;
            m_d.magnet_adc_val = iout;
            ERROR_SET(TC_EMCY_REG_CURRENT, err, &m_d);
        }else{
            ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_MAGO_MAGNET_UNDER_CURRENT);
            ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_MAGO_MAGNET_OVER_CURRENT);
        }

    }

    return err;
}

/**
 * Initial startup of the magnet.  This could also be used to reset them to a
 * known state.
 *
 * @param params pointer to the inner or outer magnet id (which magnet to init)
 * @return 0 on success or non-zero otherwise
 */
int ctrl_magnet_reinit(void)
{
    int err;
    err = mops.reinit(NULL);
    return err;
}

void ctrl_magnet_error_handler_init(void)
{
    for(uint32_t i = 0; i < SIZEOF_ARRAY(magnet_errors); i++) {
        // Check to make sure we're not trying to register duplicate error codes to fault handler
        int fault_handler_assigned = fh_fault_handlers_registry_check(magnet_errors[i].error_code);
        if(!fault_handler_assigned) {
            fh_fault_handlers_register(magnet_errors[i].error_code, magnet_errors[i].fault_handler);
        }
    }
}

int ctrl_magnet_init(void)
{
    int err;

    // Obtain semaphores - magnet only requires the command response semaphore
    cmd_rsp_semaphore = xSemaphoreCreateBinaryStatic(&cmd_rsp_semaphore_buffer);

    // register the component for boot sequence
    client_control_register_component(COMPONENT_MAGNET);

    // Initialize the magnet operations struct
    err = mops.init(&events, &cmd_rsp_semaphore);

    // register message handler callbacks - the magnet does not require a bcast_node?
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
