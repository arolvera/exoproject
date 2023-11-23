/**
 * @file    control_keeper.h
 *
 * @brief   Interface for keeper controls common to both Halo 6 and 12.
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

#ifndef CONTROL_KEEPER_H
#define CONTROL_KEEPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "definitions.h"            // Harmony definitions
#include "ext_decl_define.h"
#include "client_p.h"       // Private include for client control
#include "co_emcy.h"
#include "keeper_mcu.h"             // Shared header with Keeper MCU


    
/**
 * @brief Voltage and Current are the human readable values. Factored values
 * are ADC counts or steps.
 */
typedef struct control_keeper_t_ {
    float_t  voltage;
    uint16_t voltage_factored;
    float_t  current;
    uint16_t current_factored;
    uint8_t  ps_state;
    control_state_t status;
} control_keeper_t;
// global keeper_control
EXT_DECL control_keeper_t keeper_control;

// ToDo: are these specific to Halo 6? Do we need to define in c files?
#define DEFAULT_KEEPER_CURRENT          1.0
#define KEEPER_OVER_VOLTAGE_LIMIT       55000
#define KEEPER_OVER_VOLTAGE_WARN        50000
#define KEEPER_OVER_VOLTAGE_CLEAR       35000

/* Keeper send mask. Used to set a bitmask for the send function */
typedef enum {
    CONTROL_KEEPER_SET_VOLTS     = (1 << 0),
    CONTROL_KEEPER_SET_CURRENT   = (1 << 1),
    CONTROL_KEEPER_SET_PS_STATE  = (1 << 2),
    CONTROL_KEEPER_SET_ALL       =((1 << 3) - 1),
} ctrl_keeper_cmd_mask_t;

typedef int  (*keeper_op_init_t)    (event_callback_t *, SemaphoreHandle_t *);
typedef int  (*keeper_op_reinit_t) (void *);
typedef int  (*keeper_op_state_bcast_t)   (uint16_t, uint8_t *, can_dlc_t);
typedef int  (*keeper_op_running_t) (void);
typedef bool (*keeper_op_has_err_t) (void);
typedef int  (*keeper_op_send_t)    (ctrl_keeper_cmd_mask_t);

//keeper devops def
typedef struct keeper_ops {
    keeper_op_init_t        init;
    keeper_op_reinit_t      reinit;
    keeper_op_state_bcast_t state_bcast;
    keeper_op_running_t     is_running;
    keeper_op_has_err_t     has_error;
    keeper_op_send_t        send;
} keeper_ops_t;
//global kops
EXT_DECL keeper_ops_t kops;



/**
 * @brief Check if the keeper is running.
 * @return 0 for not running, non-zero if it is running
 */
int ctrl_keeper_isrunning(void);

/**
 * @brief Get the firmware version.
 * ToDo: replace this in Halo 12?
 * @param version Output variable to write the version to
 * @return 0 = success, non-zero otherwise
 */
int ctrl_keeper_version_get(uint32_t** version);

/**
 * @brief Convert current to ADC counts.
 * @param current Current level
 * @return ADC counts
 */
uint16_t ctrl_keeper_amperes_to_counts(float amperes);

/**
 * @brief Convert ADC counts to current.
 * @param counts ADC counts
 * @return Current level
 */
float ctrl_keeper_counts_to_amperes(uint16_t counts);

/**
 * @brief Convert input voltage to ADC counts.
 * @param voltage Voltage level
 * @return ADC counts
 */
uint16_t ctrl_keeper_input_volts_to_counts(float volts);

/**
 * @brief Convert output voltage to ADC counts.
 * @param voltage Voltage level
 * @return ADC counts
 */
uint16_t ctrl_keeper_output_volts_to_counts(float volts);

/**
 * @brief Convert input ADC counts to actual voltage levels
 * @param counts ADC counts
 * @return Keeper voltage
 */
float ctrl_keeper_input_counts_to_volts(uint16_t counts);


/**
 * @brief Convert output ADC counts to actual voltage levels
 * @param counts ADC counts
 * @return Keeper voltage
 */
/* @FIXME this needs to be private between the specialization and generalization */
float ctrl_keeper_output_counts_to_volts(uint16_t counts);

/**
 * @brief Get the actual input voltage measurement from telemetry.
 * @return Voltage value
 */
float ctrl_keeper_v_in_get(void);

/**
 * @brief Get the actual output voltage measurement from telemetry.
 * @return Voltage value
 */
float ctrl_keeper_v_out_get(void);

/**
 * @brief Get the state of the keeper power supply.
 * @return COMMANDED_ON or COMMANDED_OFF
 */
uint8_t ctrl_keeper_ps_state_get();

/**
 * @brief Set the state of the keeper power supply
 * @param on COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_ps_state_set(uint8_t on);

/**
 * @brief Get the on/off state of the Keeper.
 * @return keeper state (COMMANDED_ON or COMMANDED_OFF?)
 */
uint16_t ctrl_keeper_state_get();

/**
 * @brief Get the Keeper State Status. State status is the reason it got into
 * the state it is in. For example, the state could be 0xff which is an error.
 * The State Status might then be under-voltage or over-current.
 * @return keeper state status
 */
uint16_t ctrl_keeper_state_stat_get();

/**
 * @brief Get the keeper current value. This is the value that is SET. The actual
 * current value comes back from the client in the HSI message.
 * @return Keeper current
 */
float_t ctrl_keeper_cur_get();

/**
 * @brief Get the factored current in counts.
 * @return Factored keeper current
 */
uint16_t ctrl_keeper_cur_factored_get();

/**
 * @brief Set the keeper current.
 * @param current Current in amps
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_cur_set(float_t current);

/**
 * @brief Get the keeper voltage value. This is the value that is SET. The actual
 * voltage value comes back from the client in the HSI message.
 * @return Keeper voltage in volts
 */
float_t ctrl_keeper_volts_get();

/**
 * @brief Get the factored voltage in counts.
 * @return Voltage in ADC counts
 */
uint16_t ctrl_keeper_volts_factored_get();

/**
 * @brief Set the keeper voltage.
 * @param voltage Voltage in volts
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_volts_set(float_t voltage);

/**
 * @brief Start the keeper and wait for a spark detect (or an error that will
 * cancel the operation).
 * @param timeout maximum amount of time to wait for a result from the keeper
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_start(uint32_t timeout);

/**
 * @brief Initial startup - called whenever a new boot is detected
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_reinit(void);

/**
 * @brief Init the keeper error handling.
 */
void ctrl_keeper_error_handler_init(void);

/**
 * @brief When the keeper gets (certain) errors, it has a countdown timer that
 * will prevent you from running it again. That timer will send out an error
 * clear message when it expires and everything is back to a normal state.
 * @param ms time to wait in milliseconds
 * @return 0 on success (error clear), non-zero if the timeout happened before
 * the error cleared
 */
int ctrl_keeper_wait_err_clear(int ms);

/**
 * @brief Initialize the Keeper board
 * @return 0 on success, non-zero otherwise
 */
int ctrl_keeper_init(void);



#ifdef __cplusplus
}
#endif

#endif /* CONTROL_KEEPER_H */
