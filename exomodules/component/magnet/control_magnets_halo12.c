/**
 * @file    control_magnets_halo12.c
 *
 * @brief   Implementation for halo 12 specific magnet control. Halo 6 has two magnets,
 * inner and outer, that have separately controlled coils and currents. Halo 12 has two
 * magnets as well but has just one common coil.
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
#include "health/health.h"// For health resourse structure
#include "hsi_memory.h"
#include "magnet_mcu.h"
#include "thruster_control.h"// Component Info
#include "user-setting-values/client_control_usv.h"
#include "client-control/client_error_details.h"
#include "mcu_include.h"
#include "trace/trace.h"// Trace message
#include "ext_decl_define.h"
#include "client-update-server/client_update_server.h"
#define DECLARE_GLOBALS
#include "control_magnets.h"// This module's header file

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_MAGNETS_SUBMODULE

static int ctrl_magnet_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem);
static int ctrl_magnet_halo12_reinit(void *params);
static int ctrl_magnet_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc);
static int ctrl_magnet_halo12_isrunning(void);
static int ctrl_magnet_halo12_send(ctrl_magnet_cmd_mask_t send_mask);

magnet_opts_t mops = {
    .init       = ctrl_magnet_halo12_init,
    .reinit     = ctrl_magnet_halo12_reinit,
    .state_bcast = ctrl_magnet_halo12_state_bcast_callback,
    .is_running = ctrl_magnet_halo12_isrunning,
    .send       = ctrl_magnet_halo12_send
};

#define DEFAULT_HALO12_MAGNET_CURRENT       0.0
#define DEFAULT_MAGNET_STATE                INIT_STATE
#define DEFAULT_MAGNET_STATE_REASON         INIT_COMPLETE

static SemaphoreHandle_t *cmd_rsp_semaphore = NULL;

static event_callback_t *events;
static magnet_t telemetry;

static health_table_entry_t health_response_0[] =
    DECLARE_MAGNET_HEALTH_ENTRY_0(
        &(telemetry.output_current),
        &(telemetry.output_voltage)
                                 );

static health_table_entry_t health_response_1[] =
    DECLARE_MAGNET_HEALTH_ENTRY_1(
        &(telemetry.common.current_state),
        &(telemetry.common.error_code),
        &(telemetry.pwm_output)
                                 );
static health_table_entry_t health_response_2[] =
    DECLARE_MAGNET_HEALTH_ENTRY_2(&telemetry.temperature);

static health_array magnet_health_rsp = {health_response_0,
                                         health_response_1,
                                         health_response_2};

uint16_t ctrl_magnet_halo12_i_counts(void)
{
    return telemetry.output_current;
}

static int ctrl_magnet_halo12_isrunning(void)
{
    return (magnet_control.status.reason == POWER_GOOD && magnet_control.status.state == ON_SET_POINT);
}

/**
 * Callback function to handle state changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_magnet_halo12_state_bcast_callback(uint16_t id, uint8_t *data, can_dlc_t dlc)
{
    client_service_t service;

    communication_union_t *bcast_state = (communication_union_t *)data;

    magnet_control.status.state = bcast_state->bcast_state_change.state;
    magnet_control.status.reason = bcast_state->bcast_state_change.reason;

    switch(bcast_state->bcast_state_change.state) {
        case BCAST_INITIAL_BOOT:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_INITIAL_BOOT state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            if(bcast_state->bcast_initial_boot.boot_id == COMM_ID_MAGNET){
                int client_id = id & (~BROADCAST_STATE_ID_BASE);
                cus_image_request(client_id);
            } else {
                service.cb = ctrl_magnet_halo12_reinit;
                service.params = NULL;
                client_service_queue(&service);
                client_booted_register_component(COMPONENT_MAGNET);
            }
            break;

        case BCAST_ERROR:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_ERROR state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            // ToDo: what magnet errors and what to do about them?
            break;

        case BCAST_STATE_CHANGE:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_STATE_CHANGE state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            // ToDo: what state change reasons are there for magnets and what should we do?
            break;
    }

    return 0;
}
/* Converts HSI from counts to real world values and writes those values to hsi_mem */
static void ctrl_magnet_outer_HSI_converter(uint16_t HSI_tick)
{
    if(HSI_tick == 0) {
        hsi_mem.magnet_telem.iout = (uint16_t)(ctrl_magnet_counts_to_amperes(telemetry.output_current) * 1000);
        hsi_mem.magnet_telem.vout = (uint16_t)(ctrl_magnet_counts_to_volts(telemetry.output_voltage) * 1000);
    }
    if(HSI_tick == 1) {
        hsi_mem.magnet_telem.current_state = telemetry.common.current_state;
        hsi_mem.magnet_telem.error_code = telemetry.common.error_code;
        hsi_mem.magnet_telem.pwm_output = telemetry.pwm_output;
    }
    if(HSI_tick == 2) {
        hsi_mem.magnet_telem.temperature = telemetry.temperature;
    }
}

/**
 * Send all the Magnet settings to the Magnet according the bit mask provided
 *
 * @param send_mask Magnet set mask
 * @return 0 on success non-zero otherwise
 */
static int ctrl_magnet_halo12_send(ctrl_magnet_cmd_mask_t send_mask)
{
    int err = 0;

    command_structure_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    client_msg_t msg;
    msg.cobid = COMMAND_PARAMETERS_ID_MAGNET;
    msg.data = (uint8_t *)&cmd;
    msg.dlc = CAN_DLC_8;
    msg.psem = cmd_rsp_semaphore;
    msg.timeout = CLIENT_MSG_TIMEOUT_DEFAULT;

    if(send_mask & CONTROL_MAGNET_SET_CURRENT) {
        cmd.command = SET_CURRENT;
        cmd.specifier = MAGNET;
        cmd.set_point = magnet_control.current_factored;
        err = client_cmd_send(&msg);
    }

    if(!err && (send_mask & CONTROL_MAGNET_SET_PS_STATE)) {
        cmd.command = ON_OFF;
        if(magnet_control.ps_state == COMMANDED_ON) {
            cmd.set_point = ON_SET_POINT;
        }else if(magnet_control.ps_state == COMMANDED_OFF) {
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
 * @brief Initialize the magnet_control data structure
 */
static void ctrl_magnet_halo12_init_magnet_ctrl(void)
{
    magnet_control.current = DEFAULT_HALO12_MAGNET_CURRENT;
    magnet_control.current_factored = (uint32_t)(magnet_control.current * (float)MAGNET_COUNTS_PER_AMPERE);
    magnet_control.status.state = DEFAULT_MAGNET_STATE;
    magnet_control.status.reason = DEFAULT_MAGNET_STATE_REASON;
}

static int ctrl_magnet_halo12_reinit(void *params)
{
    int err;
    ctrl_magnet_halo12_init_magnet_ctrl();

    /* The initial settings should NOT include a SYNC or other magnet  */
    ctrl_magnet_cmd_mask_t send_mask =
        CONTROL_MAGNET_SET_CURRENT |
        CONTROL_MAGNET_SET_PS_STATE;

    err = ctrl_magnet_halo12_send(send_mask);

    return err;
}

static int ctrl_magnet_halo12_init(event_callback_t *cb, SemaphoreHandle_t *sem)
{
    int err = 0;
    cmd_rsp_semaphore = sem;
    events = cb;

    ctrl_magnet_halo12_init_magnet_ctrl();
    health_mcu_register(COMPONENT_MAGNET, magnet_health_rsp, ctrl_magnet_outer_HSI_converter);

    return err;
}
