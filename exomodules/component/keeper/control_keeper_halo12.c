/**
 * @file    control_keeper_halo12.c
 *
 * @brief   Implementation for keeper controls specific to Halo 12.
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

#include "client-control/client_control.h"
#include "control_keeper.h"
#include "error_codes.h"
#include "health/health.h"
#include "hsi_memory.h"
#include "keeper_mcu.h"
#include "trace/trace.h"

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef TraceDbg
#undef TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

static int ctrl_keeper_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem);
static int ctrl_keeper_halo12_reinit(void *params);
static int ctrl_keeper_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc);
static int ctrl_keeper_halo12_isrunning(void);
static bool ctrl_keeper_halo12_has_error(void);
static int ctrl_keeper_halo12_send(ctrl_keeper_cmd_mask_t send_mask);

keeper_ops_t kops = {
    .init = ctrl_keeper_halo12_init,
    .reinit = ctrl_keeper_halo12_reinit,
    .state_bcast = ctrl_keeper_halo12_state_bcast_callback,
    .is_running = ctrl_keeper_halo12_isrunning,
    .has_error = ctrl_keeper_halo12_has_error,
    .send = ctrl_keeper_halo12_send,
};

#define DEFAULT_KEEPER_VOLTAGE 90.0
#define DEFAULT_KEEPER_POWER_SUPPLY COMMANDED_OFF
#define DEFAULT_KEEPER_STATE OFF_STATE
#define DEFAULT_KEEPER_STATE_STAT INIT_COMPLETE
#define DEFAULT_KEEPER_SPARK_TIMEOUT 10000
#define DEFAULT_KEEPER_RUN_COUNT 50
#define DEFAULT_KEEPER_START_RETRIES 5

static SemaphoreHandle_t *cmd_rsp_semaphore = NULL;

static event_callback_t *events;
static keeper_t telemetry;

static health_table_entry_t telemetry_response_0[] =
    DECLARE_KEEPER_HEALTH_ENTRY_0(&telemetry.flyback_voltage,
                                  &telemetry.starter_voltage,
                                  &telemetry.output_current);

static health_table_entry_t telemetry_response_1[] =
    DECLARE_KEEPER_HEALTH_ENTRY_1(&telemetry.common.current_state,
                                  &telemetry.common.error_code,
                                  &telemetry.pwm_output);

//static mcu_can_house_keeping_t can_hk;
static health_table_entry_t telemetry_response_2[] =
    DECLARE_KEEPER_HEALTH_ENTRY_2(&telemetry.temperature);

static health_array telemetry_rsp = {telemetry_response_0,
                                     telemetry_response_1,
                                     telemetry_response_2};



/* There is a short period of time between when the keeper gets a voltage
 * error and when it is ready to be restarted.  It will announce when it is
 * ready
 */
static bool keeper_has_error = false;
static bool ctrl_keeper_halo12_has_error(void)
{
    return keeper_has_error;
}

static int ctrl_keeper_halo12_isrunning(void)
{
    int is_running = 0;
    if(keeper_control.status.reason == SPARK_DETECTED) {
        is_running = 1;
    }
    if(is_running && !telemetry.flyback_voltage) {
        uint8_t set_state = ctrl_keeper_ps_state_get();
        if(set_state == COMMANDED_OFF) {
            is_running = 0;
            TraceDbg(TrcMsgMcuCtl,
                     "Warning: command off, sepic at zero, possible missed state. "
                     "s:%x stat:%x set:%x",
                     keeper_control.status.state, keeper_control.status.reason, set_state,0,0,0);
        }
    }
    return is_running;
}

static int ctrl_keeper_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc)
{
    client_service_t service;

    communication_union_t *bcast_state = (communication_union_t *)data;

    keeper_control.status.state = bcast_state->bcast_state_change.state;
    keeper_control.status.reason = bcast_state->bcast_state_change.reason;

    switch(bcast_state->bcast_state_change.state) {
        case BCAST_INITIAL_BOOT:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_INITIAL_BOOT state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            service.cb = ctrl_keeper_halo12_reinit;     // SETS default keeper current and voltages
            service.params = NULL;
            client_service_queue(&service);
            client_booted_register_component(COMPONENT_KEEPER);
            break;
        case BCAST_ERROR:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_ERROR state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            if(bcast_state->bcast_state_change.reason == ERROR_CLEARED) {
                keeper_has_error = 0;
            } else {
                //FIXME - JLK - is the correct reason check
                if(bcast_state->bcast_state_change.reason == KEEPER_OVER_CURRENT_ERROR ||
                   bcast_state->bcast_state_change.reason == KEEPER_OVER_VOLTAGE_ON_STATE_ERROR) {
                    keeper_has_error = 1;
                }
                events->error(bcast_state->bcast_error.adc_val);
            }
            break;
        case BCAST_STATE_CHANGE:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_STATE_CHANGE state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            if(bcast_state->bcast_state_change.reason == SPARK_DETECTED) {
                // The Keeper sparked - notify anyone waiting for that
                TraceDbg(TrcMsgAlways, "KS_CURRENT_MODE_CONTROL unlock spark detect", 0, 0, 0, 0, 0, 0);
                events->running();
            }
            break;
    }
    return 0;
}

/* Converts HSI from counts to real world values and writes those values to hsi_mem */
static void ctrl_keeper_HSI_converter(uint16_t tick)
{
    if(tick == 0) {
        hsi_mem.keeper_telem.vout = (uint32_t)(ctrl_keeper_output_counts_to_volts(telemetry.flyback_voltage) * 1000);
        hsi_mem.keeper_telem.vin = (uint16_t)(ctrl_keeper_input_counts_to_volts(telemetry.starter_voltage) * 1000);
        // if we haven't received an offset yet, or if the offset is greater than the current, set current to zero
        if((telemetry.pwm_output == 0) || (telemetry.pwm_output > telemetry.output_current)) {
            hsi_mem.keeper_telem.iout = 0;

        } else {
            hsi_mem.keeper_telem.iout =
                (uint16_t)(ctrl_keeper_counts_to_amperes(telemetry.output_current) * 1000);
        }
    }
    if(tick == 1) {
        hsi_mem.keeper_telem.current_state = telemetry.common.current_state;
        hsi_mem.keeper_telem.error_code = telemetry.common.error_code;
        hsi_mem.keeper_telem.pwm_output = telemetry.pwm_output;
    }
    if(tick == 2) {
        hsi_mem.keeper_telem.temperature = telemetry.temperature;
    }
    TraceDbg(TrcMsgMcuCtl, "t:%d vout:%d vin:%d iout:%d p:%d t:%x",
             tick,
             hsi_mem.keeper_telem.vout,
             hsi_mem.keeper_telem.vin,
             hsi_mem.keeper_telem.iout,
             hsi_mem.keeper_telem.pwm_output,
             hsi_mem.keeper_telem.temperature);
}

/**
 * Send command to the Keeper Card according the bits set in the provided bitmask
 *
 * Prior to sending there is a conversion factor applied to some the values.
 * This is to avoid doing math on the lower capability processors.
 *
 * @param send_mask bitmask with the settings to send.
 * @return 0 on success, non-zero otherwise
 */
static int ctrl_keeper_halo12_send(ctrl_keeper_cmd_mask_t send_mask)
{
    int err = 0;

    command_structure_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    client_msg_t msg;
    msg.cobid = COMMAND_PARAMETERS_ID_KEEPER;
    msg.data = (uint8_t *)&cmd;
    msg.dlc = CAN_DLC_8;
    msg.psem = cmd_rsp_semaphore;
    msg.timeout = CLIENT_MSG_TIMEOUT_DEFAULT;

    if(send_mask & CONTROL_KEEPER_SET_CURRENT) {
        TraceDbg(TrcMsgAlways, "Setting keeper current: i:%d", keeper_control.current_factored,0,0,0,0,0);
        cmd.command   = SET_CURRENT;
        cmd.set_point = keeper_control.current_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_KEEPER_SET_VOLTS)) {
        TraceDbg(TrcMsgAlways, "Setting keeper voltage: v:%d", keeper_control.voltage_factored,0,0,0,0,0);
        cmd.command   = SET_VOLTAGE;
        cmd.set_point = keeper_control.voltage_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_KEEPER_SET_PS_STATE)) {
        TraceDbg(TrcMsgAlways, "Send Keeper ON/OFF cmd: state:%d", keeper_control.ps_state,0,0,0,0,0);
        cmd.command = ON_OFF;
        if(keeper_control.ps_state == COMMANDED_ON) {
            cmd.set_point = ON_SET_POINT;
        } else if(keeper_control.ps_state == COMMANDED_OFF) {
            cmd.set_point = OFF_SET_POINT;
        } else {
            err = __LINE__;
        }
        if(!err) {
            err = client_cmd_send(&msg);
        }
    }

    return err;
}

/**
 * @brief Initialize the keeper_control data structure
 */
static void ctrl_keeper_halo12_init_keeper_ctrl(void)
{
    keeper_control.voltage          = DEFAULT_KEEPER_VOLTAGE;
    keeper_control.voltage_factored = ctrl_keeper_output_volts_to_counts(DEFAULT_KEEPER_VOLTAGE);
    keeper_control.current          = DEFAULT_KEEPER_CURRENT;
    keeper_control.current_factored = ctrl_keeper_amperes_to_counts(DEFAULT_KEEPER_CURRENT);
    keeper_control.ps_state         = DEFAULT_KEEPER_POWER_SUPPLY;
    keeper_control.status.state     = DEFAULT_KEEPER_STATE;
    keeper_control.status.reason    = DEFAULT_KEEPER_STATE_STAT;
}

static int ctrl_keeper_halo12_reinit(void *params)
{
    int err;
    ctrl_keeper_halo12_init_keeper_ctrl();
    err = ctrl_keeper_halo12_send(CONTROL_KEEPER_SET_ALL);
    return err;
}

static int ctrl_keeper_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem)
{
    cmd_rsp_semaphore = sem;
    events = cb;
    ctrl_keeper_halo12_init_keeper_ctrl();
    health_mcu_register(COMPONENT_KEEPER, telemetry_rsp, ctrl_keeper_HSI_converter);
    return 0;
}
