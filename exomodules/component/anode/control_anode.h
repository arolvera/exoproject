/**
 * @file    control_anode.h
 *
 * @brief   Interface for anode controls common to both Halo 6 and 12.
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

#ifndef CONTROL_ANODE_H
#define CONTROL_ANODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "ext_decl_define.h"
#include "client_p.h"// Private include for client control
#include "co_emcy.h"
#include "error_codes.h"
#include "error/fault_handler.h"

/* Voltage and Current are the human readable values.
 * Factored values are what is calculated for the Atmega,
 * so they do not have to do work */
typedef struct control_anode_t_ {
    float_t voltage;
    uint16_t voltage_factored;
    float_t current;
    uint16_t current_factored;
    uint8_t ps_state;
    control_state_t status;
} control_anode_t;
// global anode_control
EXT_DECL control_anode_t anode_control;

/* Anode send mask. Used to set a bitmask for the send function */
typedef enum {
    CONTROL_ANODE_SET_VOLTS = (1 << 0),
    CONTROL_ANODE_SET_CURRENT = (1 << 1),
    CONTROL_ANODE_SET_PS_STATE = (1 << 2),
    CONTROL_ANODE_SET_ALL = ((1 << 3) - 1),
} ctrl_anode_cmd_mask_t;

typedef int (*anode_op_init_t)(event_callback_t *, SemaphoreHandle_t *);
typedef int (*anode_op_reinit_t)(void *);
typedef int (*anode_op_state_bcast_t)(uint16_t, uint8_t *, can_dlc_t);
typedef int (*anode_op_running_t)(void);
typedef bool (*anode_op_has_err_t)(void);
typedef int (*anode_op_send_t)(ctrl_anode_cmd_mask_t);

// anode devops def
typedef struct anode_ops_t_ {
    anode_op_init_t init;
    anode_op_reinit_t reinit;
    anode_op_state_bcast_t state_bcast;
    anode_op_running_t is_running;
    anode_op_send_t send;
} anode_ops_t;
//global aops
EXT_DECL anode_ops_t aops;

/**
 * Check if the anode is running.
 * @return 0 for not running, non-zero if it is running
 */
int ctrl_anode_isrunning(void);

/**
 * @brief Get the firmware version.
 * ToDo: replace this in Halo 12?
 * @param version Output variable to write the version to
 * @return 0 = success, non-zero otherwise
 */
int ctrl_anode_version_get(uint32_t** version);

/**
 * @brief Convert current to ADC counts.
 * @param current Current level
 * @return ADC counts
 */
uint16_t ctrl_anode_amperes_to_counts(float current);

/**
 * @brief Convert ADC counts to current.
 * @param counts ADC counts
 * @return Current levl
 */
float ctrl_anode_counts_to_amperes(uint16_t counts);

/**
 * @brief Convert voltage to ADC counts.
 * @param voltage Voltage level
 * @return ADC counts
 */
uint16_t ctrl_anode_volts_to_counts(float voltage);

/**
 * @brief Convert ADC counts to actual voltage levels
 * @param counts ADC counts
 * @return Anode voltage
 */
float ctrl_anode_counts_to_volts(uint16_t counts);

/**
 * @brief Get the actual voltage measurement from telemetry.
 * @return Voltage value
 */
float ctrl_anode_v_out_get(void);

/**
 * @brief Get the actual current measurement from telemetry. This function also
 * has flags to report a calculated current from setpoint power and voltage values
 * for use when HaloSim is used, or
 * @return
 */
float ctrl_anode_i_out_get(void);

/**
 * @brief Get the anode power level calculated from actual current and voltage.
 * @return ANode power
 */
float ctrl_anode_p_out_get(void);

/**
 * @brief Get the state of the anode power supply.
 * @return COMMANDED_ON or COMMANDED_OFF
 */
uint8_t ctrl_anode_ps_state_get();

/**
 * @brief Set the state of the anode power supply
 * @param on COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int ctrl_anode_ps_state_set(uint8_t on);

/**
 * @brief Get the anode current value. This is the value that is SET. The actual
 * current value comes back from the client in the HSI message.
 * @return Anode current
 */
float_t ctrl_anode_cur_get();

/**
 * @brief Get the factored current in counts.
 * @return Factored anode current
 */
uint16_t ctrl_anode_cur_factored_get();

/**
 * @brief Set the anode current.
 * @param current Current in amps
 * @return 0 on success, non-zero otherwise
 */
int ctrl_anode_cur_set(float_t current);

/**
 * @brief Get the anode voltage value. This is the value that is SET. The actual
 * voltage value comes back from the client in the HSI message.
 * @return
 */
float_t ctrl_anode_volts_get();

/**
 * @brief Get the factored voltage in counts.
 * @return
 */
uint16_t ctrl_anode_volts_factored_get();

/**
 * @brief Set the anode voltage.
 * @param voltage Voltage in volts
 * @return 0 on success, non-zero otherwise
 */
int ctrl_anode_volts_set(float_t voltage);

/**
 * @brief Monitor anode current stability by confirming actual current is in range
 * of expected for three consecutive measurements. This function tests telemetry
 * measurements of current against the expected current +/- 5%, every five seconds.
 * If it gets three consecutive measurements it returns success. If it can't get
 * three consecutive within 60s then it returns an error.
 *
 * @note This is a crude but effective algorithm. However, if the anode is stable
 * but running outside of tolerance, it might be that we need to adjust gas or
 * modify the setpoint. ToDo: provide more feedback about error?
 *
 * @param target_current Target current
 * @return 0 = current stable, non-zero otherwise
 */
int ctrl_anode_monitor_stability(float_t target_current);

/**
 * @brief Start the anode (turn the boost power supply on) and wait for a spark
 * detect (or an error that will cancel the operation). If timeout is 0, then
 * simply turn on the supply and do not wait for a spark.
 * @param timeout Maximum amount of time to wait for a result from the anode
 * @return 0 on success, non-zero otherwise
 */
int ctrl_anode_start(uint32_t timeout);

/**
 * @brief Set all values to default
 * @return 0 on success, non-zero otherwise
 */
int ctrl_anode_reinit(void);

/**
 * @brief Initialize the anode error handler.
 */
void ctrl_anode_error_handler_init(void);

/**
 * @brief Initialize the Anode board
 * @return Always returns 0
 */
int ctrl_anode_init(void);



#ifdef __cplusplus
}
#endif

#endif /* CONTROL_ANODE_H */

