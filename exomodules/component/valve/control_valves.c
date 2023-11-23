/**
 * @file    control_valves.c
 *
 * @brief   This file contains the control logic, memory, and function for the
 *          Valves microcontroller.
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

#include "control_valves.h"               // This module's header file
#include "client-control/client_control.h"// Client services
#include "co_core.h"
#include "error/error_handler.h"
#include "health/health.h"// For health resourse structure
#include "hsi_memory.h"
#include "thruster_control.h"// Component IDs
#include "trace/trace.h"     // Trace message
#include "user-setting-values/calib_vals_usv.h"
#include "valve_mcu.h"
#include <math.h>
#include <string.h>
#include "hardwarecontrol_version.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_VALVES_SUBMODULE

#define DETAIL_LOG_LENGTH LOG_LENGTH_16

static int ctrl_valves_send(CONTROL_VAVLES_CMD_MASK send_mask);

typedef struct valves_specific_error {
  uint32_t valve_status;
} valves_specific_error_t;
typedef struct valves_error_detail {
  base_error_detail_t b_d;
  valves_specific_error_t valves_specific_error;
} valves_control_error_detail_t;

static valves_control_error_detail_t error_detail[DETAIL_LOG_LENGTH];

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
OSAL_MUTEX_HANDLE_TYPE mutex_valves;

static SemaphoreHandle_t cmd_rsp_semaphore;
static StaticSemaphore_t cmd_rsp_semaphore_buffer;

/* Memory for the valves control values */
control_valves_t valves_control = {
    .cathode_hf             = DEFUALT_VALVES_CATHODE_HF,
    .cathode_hf_factored    = 0,
    .cathode_lf             = DEFUALT_VALVES_CATHODE_LF,
    .cathode_lf_factored    = 0,
    .anode_flow             = DEFUALT_VALVES_ANODE_FLOW,
    .anode_flow_factored    = 0,
    .latch_valve            = DEFUALT_VALVES_LATCH_VALVE,
    .stat = {.state = INIT_STATE, .reason = INIT_COMPLETE},
};

typedef struct {
  float counts_per_low;
  float counts_per_psi_high;
  float counts_per_volt;
  uint16_t thermistor_beta;
  uint16_t thermistor_r_nought;
} valve_scaling_t;

static valve_scaling_t valve_scaling = {
    .counts_per_low = COUNTS_PER_PSI_LOW_PRESSURE,
    .counts_per_psi_high = COUNTS_PER_PSI_HIGH_PRESSURE,
    .counts_per_volt = VALVE_VOLTAGE_SCALE,
    .thermistor_beta = VALVE_THERMISTOR_BETA,
    .thermistor_r_nought = VALVE_THERMISTOR_R_NOUGHT
};

/* Give this function psi and it will return counts for the 40 psi sensor */
uint16_t ctrl_valve_psi_to_counts_forty(float psi)
{
    return (uint16_t)(psi * valve_scaling.counts_per_low);
};

/* Give this function counts and it will return psi for the 40 psi sensor */
float ctrl_valve_counts_to_psi_forty(uint16_t counts)
{
    return ((float)counts / valve_scaling.counts_per_low);
};

/* Give this function psi and it will return counts for the 3000 psi sensor */
uint16_t ctrl_valve_psi_to_counts_three_thousand(float psi)
{
    return (uint16_t)(psi * calib_vals_usv_cnt_psi_three_thousand_get());
};

/* Give this function counts and it will return psi for the 3000 psi sensor */
float ctrl_valve_counts_to_psi_three_thousand(uint16_t counts)
{
    return ((float)counts / calib_vals_usv_cnt_psi_three_thousand_get());
};

/* This function converts a 0%-99% duty cycle to a permil setpoint */
uint16_t ctrl_valve_duty_cycle_to_PSC_setpoint(uint16_t duty_cycle)
{
    if(duty_cycle > 100) {
        duty_cycle = 100;
    }
    return (duty_cycle * 10);
}

/* Give this function counts and it will return valve voltage */
float ctrl_valve_counts_to_volts(uint16_t counts)
{
    float valve_volts = 0;
    // Valve voltage is measured inverse so it needs to be subtracted from 
    // the rail voltage
    if((float)hsi_mem.hk_adc_telem.mV_14V > (((float)counts / valve_scaling.counts_per_volt) * 1000)) {
        valve_volts = ((float)hsi_mem.hk_adc_telem.mV_14V -
            (((float)counts / valve_scaling.counts_per_volt) * 1000));
        valve_volts = valve_volts / 1000;   // convert back to V from mV
    }
    return valve_volts;
}

/* Give this function counts and it will return temperature in K */
float ctrl_valve_counts_to_temperature(uint16_t counts)
{
    float therm_resistance;
    float temperature;
    float fixed_resistance = 2200;
    float v_ref = (float)2.56;
    float v_excitation = (float)3.3;
    uint16_t adc_range = 1023;
    float t_nought = (float)298.15;    // 25 degrees C

    /* TODO: Who'd-a-thunk it would be so hard to calculate temperature? I'm 
     * going to leave this as is for now, but if we need to stop including the 
     * math library we can write our own exp() and log() functions. -FN */
    therm_resistance = (-1 * fixed_resistance * v_ref * (float)counts) /
        (((float)counts * v_ref) - ((float)adc_range * v_excitation));

    temperature = (float)valve_scaling.thermistor_beta / (float)(log((double)(therm_resistance /
        (valve_scaling.thermistor_r_nought *
            exp((double)((-1 * (float)valve_scaling.thermistor_beta) / t_nought))))));

    return temperature;
}

static valve_t valve_hsi;

static health_table_entry_t health_response_0[] =
    DECLARE_VALVE_HEALTH_ENTRY_0(&(valve_hsi.tank_pressure),
                                 &(valve_hsi.cathode_pressure),
                                 &(valve_hsi.anode_pressure),
                                 &(valve_hsi.regulator_pressure));

static health_table_entry_t health_response_1[] =
    DECLARE_VALVE_HEALTH_ENTRY_1(&(valve_hsi.common.current_state),
                                 &(valve_hsi.common.error_code),
                                 &(valve_hsi.anode_dac),
                                 &(valve_hsi.cat_low_flow_dac));

//static mcu_can_house_keeping_t can_hk;
static health_table_entry_t health_response_2[] =
    DECLARE_VALVE_HEALTH_ENTRY_2(&(valve_hsi.temperature),
                                 &(valve_hsi.cat_high_flow_voltage));

static health_array valves_health_rsp = {
    health_response_0,
    health_response_1,
    health_response_2
};

/* Call back for handling command responses */
static int ctrl_valves_rsp_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t rsp_node = {
    .node = {
        .range_low  = RESPONSE_PARAMETERS_ID_VALVE,
        .range_high = RESPONSE_PARAMETERS_ID_VALVE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_valves_rsp_callback,
};

/* Callback function to handle broadcast messages */
static int ctrl_valves_state_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t bcast_node = {
    .node = {
        .range_low  = BROADCAST_STATE_ID_VALVE,
        .range_high = BROADCAST_STATE_ID_VALVE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_valves_state_bcast_callback,
};

/* Call back function to handle valve variable changes */
static int ctrl_valves_variable_bcast_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
static msg_callback_t variable_node = {
    .node = {
        .range_low  = BROADCAST_VARIABLE_ID_VALVE,
        .range_high = BROADCAST_VARIABLE_ID_VALVE,
        .left = NULL,
        .right = NULL,
    },
    .cb = ctrl_valves_variable_bcast_callback,
};

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/**
 * Send the initial startup values to the valves.  This is called when the valves
 * is first powered on, but could also be used to 'reset' it to a known state
 * @param params not used
 * @return 0 on success, non-zero otherwise
 */
static int ctrl_valves_startup(void *params)
{
    int err;
    /* Belt and suspenders, this 'should' be called at initialization, and this
   already structure defaulted when declared globally above, but just in
   case this gets used as a 're-init' function, default the variables. */

    valves_control.cathode_hf = DEFUALT_VALVES_CATHODE_HF;
    valves_control.cathode_hf_factored =
        ctrl_valve_duty_cycle_to_PSC_setpoint(DEFUALT_VALVES_CATHODE_HF);

    valves_control.cathode_lf = DEFUALT_VALVES_CATHODE_LF;
    valves_control.cathode_lf_factored =
        ctrl_valve_psi_to_counts_forty(DEFUALT_VALVES_CATHODE_LF);

    valves_control.anode_flow = DEFUALT_VALVES_ANODE_FLOW;
    valves_control.anode_flow_factored =
        ctrl_valve_psi_to_counts_forty(DEFUALT_VALVES_ANODE_FLOW);

    valves_control.latch_valve = DEFUALT_VALVES_LATCH_VALVE;

    err = ctrl_valves_send(CONTROL_VALVES_SET_ALL);

    return err;
}
/**
 * Callback function to handle command responses
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_valves_rsp_callback(message_t *msg)
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
static version_info_t app_version;
static int ctrl_valves_state_bcast_callback(message_t *msg)
{
    uint8_t *data = msg->data;
    uint16_t id = msg->id;
    client_service_t service;

    communication_union_t *bcast_state = (communication_union_t *)data;

    valves_control.stat.state = bcast_state->bcast_state_change.state;
    valves_control.stat.reason = bcast_state->bcast_state_change.reason;

    switch(bcast_state->bcast_state_change.state) {
        case BCAST_INITIAL_BOOT:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_INITIAL_BOOT state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            service.cb = ctrl_valves_startup;
            service.params = NULL;
            client_service_queue(&service);
            client_booted_register_component(COMPONENT_VALVES);
            break;

        case BCAST_ERROR:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_ERROR state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            /* Notify the sequencer of the error */
            client_error_check(ABORT_VALVE_ERROR);
            ERROR_SET(ERROR_CODE_VALVE_COMM_UNKNOWN_COMMAND_ERROR, CO_EMCY_REG_COM, NULL);
            break;

        case BCAST_STATE_CHANGE:
            TraceInfo(TrcMsgMcuCtl, " id:%x type:BCAST_STATE_CHANGE state:%02x reason:%02x",
                      id, *((uint8_t *)(&data[1])), *((uint8_t *)(&data[2])), 0, 0, 0);
            // ToDo: what state changes can we expect from the valves? Power supply?
    }

    return 0;
}

/**
 * Callback function to handle variable changes
 * @param id message ID
 * @param data pointer to message data
 * @param dlc length of data message
 * @return  0
 */
static int ctrl_valves_variable_bcast_callback(message_t *msg)
{
    int err = 0;
    uint8_t *data = msg->data;
    uint16_t id = msg->id;

    variable_type_t *variable_type = (variable_type_t *)data;

    if(*variable_type == SCALING_INFO) {
        bcast_scaling_t *bcast_scaling = (bcast_scaling_t *)data;

        TraceInfo(TrcMsgMcuCtl, " id:%x type:SCALING_INFO unit:%d factor:0x%02x%02x%02x%02x",
                  id,bcast_scaling->unit_description,data[7],data[6],data[5],data[4]);

        switch(bcast_scaling->unit_description) {
            //FIXME - JLK - is this correct
            case COUNTS_PER_PSI_LOW_PRESSURE:
                valve_scaling.counts_per_low = bcast_scaling->scaling;
                break;

            case COUNTS_PER_PSI_HIGH_PRESSURE:
                calib_vals_usv_cnt_psi_three_thousand_set(bcast_scaling->scaling);
                break;

            case COUNTS_PER_VOLT_OUTPUT:
                valve_scaling.counts_per_volt = bcast_scaling->scaling;
                break;

            case THERMISTOR_BETA:
                valve_scaling.thermistor_beta = bcast_scaling->scaling;
                break;

            case THERMISTOR_R_NOUGHT:
                valve_scaling.thermistor_r_nought = bcast_scaling->scaling;
                break;

            default:
                err = __LINE__;
                break;
        }
    }else if(*variable_type == VERSION_INFO) {
        mcu_version_t *mcu_version = (mcu_version_t *)data;

        app_version.major = mcu_version->major;
        app_version.minor = mcu_version->minor;
        app_version.rev = mcu_version->rev;
        app_version.git_sha = *((uint32_t *)&mcu_version->git_sha);

        TraceInfo(TrcMsgMcuCtl, " id:%x type:VERSION_INFO maj:%02x min:%02x rev:%02x sha:%08x",
                  id,
                  *((uint8_t *)(&app_version.major)),
                  *((uint8_t *)(&app_version.minor)),
                  *((uint8_t *)(&app_version.rev)),
                  *((uint32_t *)(&app_version.git_sha)),
                  0);
    }else {
        err = __LINE__;
    }

    return err;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
int ctrl_valves_version_get(uint32_t **version)
{
    int err = 0;
    if(version == NULL) {
        err = __LINE__;
    }else {
        version[0] = (uint32_t *)&app_version;
        version[1] = (uint32_t*)&app_version.git_sha;
    }
    return err;
}

uint16_t ctrl_valves_cath_pressure(void)
{
    return hsi_mem.valves_telem_2.cathode_pressure;
}

/*
 * Strategy for error handling
 */
static void *ctrl_valves_error_detail_strategy(void *arg)
{
    if(arg != NULL) {
        EH_LOG_ERROR(valves_specific_error, arg);
    }
    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}

/**
 * Set the Cathode High Flow PSI - PSI will be converted to the value the 
 * needed by the Valve MCU and stored in the factored field
 * @param psi high flow setting in PSI
 * @return 0 on success or -1 on failure
 */
int ctrl_valves_cathode_hf_set(float psi)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    valves_control.cathode_hf = psi;
    valves_control.cathode_hf_factored =
        ctrl_valve_duty_cycle_to_PSC_setpoint(psi);
    err = ctrl_valves_send(CONTROL_VALVES_CATHODE_HF);
    OSAL_MUTEX_Unlock(&mutex_valves);
    return err;
}

/**
 * Get Cathode High Flow
 * @return Cathode High Flow
 */
float ctrl_valves_cathode_hf_get()
{
    float ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.cathode_hf;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Get the factored Cathode High Flow value.  This was calculated when the
 * PSI was set.
 * @return factored Cathode High Flow value.
 */
uint16_t ctrl_valves_cathode_hf_factored_get()
{
    uint16_t ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.cathode_hf_factored;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Set the Cathode Low Flow PSI - PSI will be converted to the value the 
 * needed by the Valve MCU and stored in the factored field
 * @param psi low flow setting in PSI
 * @return 0 on success or -1 on failure
 */
int ctrl_valves_cathode_lf_set(float psi)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    valves_control.cathode_lf = psi;
    valves_control.cathode_lf_factored =
        ctrl_valve_psi_to_counts_forty(psi);
    err = ctrl_valves_send(CONTROL_VALVES_CATHODE_LF);
    OSAL_MUTEX_Unlock(&mutex_valves);
    return err;
}

/**
 * Get Cathode Low Flow
 * @return Cathode Low Flow
 */
float ctrl_valves_cathode_lf_get()
{
    float ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.cathode_lf;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Get the factored Cathode Low Flow value.  This was calculated when the
 * PSI was set.
 * @return factored Cathode Low Flow value.
 */
uint16_t ctrl_valves_cathode_lf_factored_get()
{
    uint16_t ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.cathode_lf_factored;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Set the Anode Flow PSI - PSI will be converted to the value the 
 * needed by the Valve MCU and stored in the factored field
 * @param psi Anode flow setting in PSI
 * @return 0 on success or -1 on failure
 */
int ctrl_valves_anode_flow_set(float psi)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    valves_control.anode_flow = psi;
    valves_control.anode_flow_factored =
        ctrl_valve_psi_to_counts_forty(psi);
    err = ctrl_valves_send(CONTROL_VALVES_ANODE_FLOW);
    OSAL_MUTEX_Unlock(&mutex_valves);
    return err;
}

/**
 * Get the Anode Flow
 * @return Anode Flow
 */
float ctrl_valves_anode_flow_get()
{
    float ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.anode_flow;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Get the factored Anode Flow value.  This was calculated when the
 * PSI was set.
 * @return factored Anode Flow value.
 */
uint16_t ctrl_valves_anode_flow_factored_get()
{
    uint16_t ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.anode_flow_factored;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Get the Valve setting, copy to provided structure
 * @param c pointer to store valve settings
 * @return 0 on success or -1 on failure
 */
int ctrl_valves_get(control_valves_t *c)
{
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    memcpy(c, &valves_control, sizeof(control_valves_t));
    OSAL_MUTEX_Unlock(&mutex_valves);
    return 0;
}

/**
 * Open/Close the latch valve
 * @param open 0x2a = ON, 0x15 = off
 * @return 0 on success or -1 on failure
 */
int ctrl_valves_latch_valve_set(uint8_t open)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    if(open == ON_SET_POINT || open == OFF_SET_POINT) {
        valves_control.latch_valve = open;
        err = ctrl_valves_send(CONTROL_VALVES_LATCH_VALVE);
    }else {
        err = -1;
    }
    OSAL_MUTEX_Unlock(&mutex_valves);
    return err;
}

/**
 * Get the latch valve setting
 * @return latch valve setting
 */
uint8_t ctrl_valves_latch_valve_get()
{
    uint8_t ret;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    ret = valves_control.latch_valve;
    OSAL_MUTEX_Unlock(&mutex_valves);
    return ret;
}

/**
 * Set the setting in provided structure
 * @param c pointer to valve settings
 * @return 0 on success or -1 on error
 */
int ctrl_valves_set(control_valves_t *c)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_valves, OSAL_WAIT_FOREVER);
    memcpy(&valves_control, c, sizeof(control_valves_t));
    err = ctrl_valves_send(CONTROL_VALVES_SET_ALL);
    OSAL_MUTEX_Unlock(&mutex_valves);
    return err;
}

/* Converts HSI from counts to real world values and writes those values to hsi_mem */
static void ctrl_valve_HSI_converter(uint16_t HSI_tick)
{
    if(HSI_tick == 0) {
        //        hsi_mem.valves_telem_1.anode_v =
        //            (uint16_t)(ctrl_valve_counts_to_volts(valves_hsi_1_counts.anode_v) * 1000);
        //        hsi_mem.valves_telem_1.cathode_hf_v =
        //            (uint16_t)(ctrl_valve_counts_to_volts(valves_hsi_1_counts.cathode_hf_v) * 1000);
        //        hsi_mem.valves_telem_1.cathode_lf_v =
        //            (uint16_t)(ctrl_valve_counts_to_volts(valves_hsi_1_counts.cathode_lf_v) * 1000);
        //        hsi_mem.valves_telem_1.temperature =
        //            (int32_t)((ctrl_valve_counts_to_temperature(valves_hsi_1_counts.temperature) - 273.15) * 1000);

    }
    if(HSI_tick == 1) {
        //        hsi_mem.valves_telem_2.tank_pressure =
        //            (uint32_t)(ctrl_valve_counts_to_psi_three_thousand(valves_hsi_2_counts.tank_pressure) * 1000);
        //        hsi_mem.valves_telem_2.cathode_pressure =
        //            (uint16_t)(ctrl_valve_counts_to_psi_forty(valves_hsi_2_counts.cathode_pressure) * 1000);
        //        hsi_mem.valves_telem_2.anode_pressure =
        //            (uint16_t)(ctrl_valve_counts_to_psi_forty(valves_hsi_2_counts.anode_pressure) * 1000);
        //        hsi_mem.valves_telem_2.regulator_pressure =
        //            (uint16_t)(ctrl_valve_counts_to_psi_forty(valves_hsi_2_counts.regulator_pressure) * 1000);
    }
    if(HSI_tick == 2) {
        //        hsi_mem.valves_telem_3.can_err = valves_hsi_3_counts.can_err;
        /*
         * @TODO Add Fault Handler here - @FIXME - lose the OR - Equals and save error properly!
         */
        //        hsi_mem.valves_telem_3.can_msg_cnt |= valves_hsi_3_counts.can_msg_cnt;
    }
}

/**
 * Send command to the Valve Card according the provided bitmask
 *
 * @return 0 on success, non-zero otherwise
 */
int ctrl_valves_send(CONTROL_VAVLES_CMD_MASK send_mask)
{
    int err = 0;

    command_structure_t cmd;
    memset(&cmd, 0, sizeof(cmd));

    client_msg_t msg;
    msg.cobid = COMMAND_PARAMETERS_ID_VALVE;
    msg.data = (uint8_t *)&cmd;
    msg.dlc = CAN_DLC_8;
    msg.psem = &cmd_rsp_semaphore;
    msg.timeout = CLIENT_MSG_TIMEOUT_DEFAULT;

    if(send_mask & CONTROL_VALVES_CATHODE_HF) {
        cmd.command = SET_FLOW;
        cmd.specifier = CAT_HIGH_FLOW;
        cmd.set_point = valves_control.cathode_hf_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_VALVES_CATHODE_LF)) {
        cmd.command = SET_FLOW;
        cmd.specifier = CAT_LOW_FLOW;
        cmd.set_point = valves_control.cathode_lf_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_VALVES_ANODE_FLOW)) {
        cmd.command = SET_FLOW;
        cmd.specifier = ANODE_FLOW;
        cmd.set_point = valves_control.anode_flow_factored;
        err = client_cmd_send(&msg);
    }
    if(!err && (send_mask & CONTROL_VALVES_LATCH_VALVE)) {
        cmd.command = ON_OFF;
        cmd.set_point = valves_control.latch_valve;
        err = client_cmd_send(&msg);
    }
    return err;
}

/**
 * Set all values to default
 * @return 0 on success, non-zero otherwise
 */
int ctrl_valves_reinit(void)
{
    return ctrl_valves_startup(NULL);
}

/*
 * Init the anodes error handling
 */
void ctrl_valves_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, ctrl_valves_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);
    fh_fault_handlers_register(ERROR_CODE_VALVE_COMM_UNKNOWN_COMMAND_ERROR, FH_ALERT);
}

/**
 * Initialize the valves
 * @return  0 on success or -1 on failure
 */
int ctrl_valves_init(void)
{
    // create a mutex for the valves - other components do this in the command_<comp>.c file
    static StaticSemaphore_t xMutexBuffer;
    mutex_valves = xSemaphoreCreateMutexStatic(&xMutexBuffer);

    // Obtain semaphores - valves only require the command response semaphore
    cmd_rsp_semaphore = xSemaphoreCreateBinaryStatic(&cmd_rsp_semaphore_buffer);

    // register the component for boot sequence
    client_control_register_component(COMPONENT_VALVES);
    health_mcu_register(COMPONENT_VALVES, valves_health_rsp, ctrl_valve_HSI_converter);

    // Should we define a valves operations struct like the other components?

    // register message handler callbacks
    msg_handler_register_callback(&rsp_node);
    msg_handler_register_callback(&bcast_node);
    msg_handler_register_callback(&variable_node);

    // ToDo: how to properly initialize app version
    app_version.rev = (uint8_t)HW_REVISION;             // TC_REVISION   HW_REVISION
    app_version.minor = (uint8_t)HW_MINOR_VERSION;      // TC_MINOR_VERSION     HW_MINOR_VERSION
    app_version.major = (uint8_t)HW_MAJOR_VERSION;      // TC_MAJOR_VERSION     HW_MAJOR_VERSION
    app_version.reserved = 0;
    app_version.git_sha = (uint32_t)HW_GIT_SHA;          // HW_GIT_SHA     HW_GIT_SHA

    return 0;
}