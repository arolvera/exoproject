/**
 * @file    control_anode_halo12.c
 *
 * @brief   Implementation for halo 12 specific anode control.
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

#include "anode_mcu.h"
#include "client-control/client_service.h"
#include "health/health.h"
#include "hsi_memory.h"
#include "trace/trace.h"// Trace message
#include "utils/macro_tools.h"

#include "ext_decl_define.h"
#define DECLARE_GLOBALS
#include "control_anode.h"
#include "client-update-server/client_update_server.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

static int ctrl_anode_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem);
static int ctrl_anode_halo12_reinit(void *params);
static int ctrl_anode_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc);
static int ctrl_anode_halo12_isrunning(void);
static int ctrl_anode_halo12_send(ctrl_anode_cmd_mask_t send_mask);

anode_ops_t aops = {
    .init       = ctrl_anode_halo12_init,
    .reinit     = ctrl_anode_halo12_reinit,
    .state_bcast = ctrl_anode_halo12_state_bcast_callback,
    .is_running = ctrl_anode_halo12_isrunning,
    .send       = ctrl_anode_halo12_send,
};

#define DEFUALT_ANODE_VOLTAGE           100.0
#define DEFUALT_ANODE_CURRENT           1.0
#define DEFUALT_ANODE_POWER_SUPPLY      COMMANDED_OFF
#define DEFUALT_ANODE_STATE             OFF_STATE
#define DEFUALT_ANODE_STATE_REASON      INIT_COMPLETE

static SemaphoreHandle_t *cmd_rsp_semaphore = NULL;

static event_callback_t *events;
static anode_t telemetry;

static health_table_entry_t health_response_0[] =
    DECLARE_ANODE_HEALTH_ENTRY_0(&(telemetry.output_voltage),
                                 &(telemetry.output_current),
                                 &(telemetry.x_voltage),
                                 &(telemetry.y_voltage));

static health_table_entry_t health_response_1[] =
    DECLARE_ANODE_HEALTH_ENTRY_1(&(telemetry.common.current_state),
                                 &(telemetry.common.error_code),
                                 &(telemetry.x_pwm_output),
                                 &(telemetry.y_pwm_output),
                                 &(telemetry.mode));

//static mcu_can_house_keeping_t can_hk;
static health_table_entry_t health_response_2[] =
    DECLARE_ANODE_HEALTH_ENTRY_2(&(telemetry.temperature),
                                 &(telemetry.raw_input_voltage),
                                 &(telemetry.filtered_input_voltage));

static health_array anode_health_rsp = {health_response_0,
                                        health_response_1,
                                        health_response_2};

/**
 * Check if the anode is running.
 * @return 0 for not running, non-zero if it is running
 */
static int ctrl_anode_halo12_isrunning(void)
{
    return anode_control.status.reason == SPARK_DETECTED;
}

/**
 * Callback function to handle state changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_anode_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc)
{
    client_service_t service;

    communication_union_t *bcast_state = (communication_union_t *)data;

    anode_control.status.state = bcast_state->bcast_state_change.state;
    anode_control.status.reason = bcast_state->bcast_state_change.reason;

    switch(bcast_state->bcast_state_change.state) {
        case BCAST_INITIAL_BOOT:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_INITIAL_BOOT state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            if(bcast_state->bcast_initial_boot.boot_id == COMM_ID_ANODE){
                int client_id = id & (~BROADCAST_STATE_ID_BASE);
                cus_image_request(client_id);
            } else  {
                service.cb = ctrl_anode_halo12_reinit;
                service.params = NULL;
                client_service_queue(&service);
                client_booted_register_component(COMPONENT_ANODE);
            }
            break;

        case BCAST_ERROR:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_ERROR state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            events->error(bcast_state->bcast_error.adc_val);
            break;

        case BCAST_STATE_CHANGE:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_STATE_CHANGE state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            if(bcast_state->bcast_state_change.reason == SPARK_DETECTED) {
                TraceDbg(TrcMsgSeq, "ANODE plasma burning", 0, 0, 0, 0, 0, 0);
                events->running();
            }
            break;
    }

    return 0;
}

/* Converts HSI from counts to real world values and writes those values to hsi_mem */
static void ctrl_anode_HSI_converter(uint16_t HSI_tick)
{
    if(HSI_tick == 0) {
        hsi_mem.anode_telem.vout = (uint32_t)(ctrl_anode_counts_to_volts(telemetry.output_voltage) * 1000);
        hsi_mem.anode_telem.iout = (uint16_t)(ctrl_anode_counts_to_amperes(telemetry.output_current) * 1000);
        hsi_mem.anode_telem.v_x = telemetry.x_voltage;
        hsi_mem.anode_telem.v_y = telemetry.y_voltage;
    }
    if(HSI_tick == 1) {
        hsi_mem.anode_telem.current_state = telemetry.common.current_state;
        hsi_mem.anode_telem.error_code = telemetry.common.error_code;
        hsi_mem.anode_telem.x_pwm_output = telemetry.x_pwm_output;
        hsi_mem.anode_telem.y_pwn_output = telemetry.y_pwm_output;
    }
    if(HSI_tick == 2) {
        hsi_mem.anode_telem.temperature = telemetry.temperature;
        hsi_mem.anode_telem.raw_input_voltage = telemetry.raw_input_voltage;
        hsi_mem.anode_telem.raw_input_voltage = telemetry.raw_input_voltage;
    }
};

/**
 * Send command to the Anode Card according the bits set in the provided bitmask
 *
 * Prior to sending there is a conversion factor applied to some the values.
 * This is to avoid doing math on the lower capability processors.
 *
 * @param send_mask bitmask with the settings to send.
 * @return 0 on success, non-zero otherwise
 */
static int ctrl_anode_halo12_send(ctrl_anode_cmd_mask_t send_mask)
{
    int err = 0;

    command_structure_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    client_msg_t msg;
    msg.cobid = COMMAND_PARAMETERS_ID_ANODE;
    msg.data = (uint8_t *)&cmd;
    msg.dlc = CAN_DLC_8;
    msg.psem = cmd_rsp_semaphore;
    msg.timeout = CLIENT_MSG_TIMEOUT_DEFAULT;

    if(send_mask & CONTROL_ANODE_SET_CURRENT) {
        cmd.command = SET_CURRENT;
        cmd.set_point = anode_control.current_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_ANODE_SET_VOLTS)) {
        cmd.command = SET_VOLTAGE;
        cmd.set_point = anode_control.voltage_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_ANODE_SET_PS_STATE)) {
        cmd.command = ON_OFF;
        if(anode_control.ps_state == COMMANDED_ON) {
            cmd.set_point = ON_SET_POINT;
        }else if(anode_control.ps_state == COMMANDED_OFF) {
            cmd.set_point = OFF_SET_POINT;
        }else {
            err = __LINE__;
        }
        if(!err) {
            err = client_cmd_send(&msg);
        }
    }

    return err;
}

/**
 * @brief Initialize the anode_control data structure
 */
static void ctrl_anode_halo12_init_anode_ctrl(void)
{
    anode_control.voltage              = DEFUALT_ANODE_VOLTAGE;
    anode_control.voltage_factored     = ctrl_anode_volts_to_counts(DEFUALT_ANODE_VOLTAGE);
    anode_control.current              = DEFUALT_ANODE_CURRENT;
    anode_control.current_factored     = ctrl_anode_amperes_to_counts(DEFUALT_ANODE_CURRENT);
    anode_control.ps_state             = DEFUALT_ANODE_POWER_SUPPLY;
    anode_control.status.state         = DEFUALT_ANODE_STATE;
    anode_control.status.reason        = DEFUALT_ANODE_STATE_REASON;
}

static int ctrl_anode_halo12_reinit(void *params)
{
    int err;
    ctrl_anode_halo12_init_anode_ctrl();
    err = ctrl_anode_halo12_send(CONTROL_ANODE_SET_ALL);
    return err;
}

static int ctrl_anode_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem)
{
    cmd_rsp_semaphore = sem;
    events = cb;
    ctrl_anode_halo12_init_anode_ctrl();
    health_mcu_register(COMPONENT_ANODE, anode_health_rsp, ctrl_anode_HSI_converter);
    return 0;
}
