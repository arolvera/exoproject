/**
 * @file    control_magnets.h
 *
 * @brief   Interface for magnet controls.
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

#ifndef CONTROL_MAGNET_H
#define CONTROL_MAGNET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "definitions.h"            // Harmony definitions
#include "client_p.h"       // Private include for client control
#include "component_type.h"
#include "co_core.h"
#include "error_codes.h"
#include "fault_handler.h"



/**
 * @brief Magnet send mask. Used to set a bitmask for the send function
 */
typedef enum {
    CONTROL_MAGNET_SET_CURRENT      = (1 << 1),
    CONTROL_MAGNET_SET_PS_STATE     = (1 << 2),
    CONTROL_MAGNET_SET_ALL          = ((1 << 3) - 1),
} ctrl_magnet_cmd_mask_t;

/**
 * @brief control_magnets_t is a structure that holds information about the settings
 * for the magnets. Current and ratio are the human readable values. Factored values
 * reflect the ADC resolution in steps or counts.
 */
typedef struct control_magnets_t_ {
    float    current;
    uint16_t current_factored;
    uint8_t ps_state;
    control_state_t status;
} control_magnets_t;
// global magnet control
EXT_DECL control_magnets_t magnet_control;

/**
 * @brief The magnet_ops_t structure abstracts operations that are common to each type
 * of thruster but require different implementations.
 */
typedef int (*magnet_op_init_t) (event_callback_t *, SemaphoreHandle_t *);
typedef int (*magnet_op_reinit_t) (void *);
typedef int (*magnet_op_state_bcast_t) (uint16_t, uint8_t *, can_dlc_t);
typedef int (*magnet_op_running_t) (void);
typedef int (*magnet_op_send_t) (ctrl_magnet_cmd_mask_t);

typedef struct magnet_ops_t_ {
    magnet_op_init_t        init;
    magnet_op_reinit_t      reinit;
    magnet_op_state_bcast_t state_bcast;
    magnet_op_running_t     is_running;
    magnet_op_send_t        send;
} magnet_opts_t;
//global mops
EXT_DECL magnet_opts_t mops;

uint16_t ctrl_magnet_halo12_i_counts(void);

/**
 * @brief Convert volts to counts.
 * @param volts
 * @return counts
 */
uint16_t ctrl_magnet_volts_to_counts(float volts);

/**
 * @brief Convert counts to volts.
 * @param counts
 * @return voltage
 */
float ctrl_magnet_counts_to_volts(uint16_t counts);

/**
 * @brief Convert amperes to counts.
 * @param amperes
 * @return counts
 */
uint16_t ctrl_magnet_amperes_to_counts(float amperes);

/**
 * @brief Convert counts to amperes.
 * @param counts
 * @return amperes
 */
float ctrl_magnet_counts_to_amperes(uint16_t counts);

/**
 * @brief Get the current on/off state of the magnets.
 * @return State value
 */
uint8_t ctrl_magnet_ps_state_get();

/**
 * @brief Turn the magnets on or off.
 * @param state COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int ctrl_magnet_ps_state_set(uint8_t state);

/**
 * @brief Read the firmware version of the magnet controller.
 * @param version Pointer to a firmware version object to write the result to.
 * @return
 */
int ctrl_magnets_version_get(uint32_t** version);

/**
 * @brief Set the magnet Current.
 * @param current current setting
 * @return 0 on success, non-zero otherwise
 */
int ctrl_magnet_current_set(float current);

/**
 * @brief Get the magnet current setting.
 * @return Ampere version of current.
 */
float ctrl_magnet_current_get();

/**
 * @brief Get the factored magnet current setting.
 * @return Counts version of current.
 */
uint16_t ctrl_magnet_current_factored_get();

/**
 * @brief Check that the magnet current is in an acceptable range according to the current
 * set point.
 * @return 0 if in range, non-zero otherwise
 */
int ctrl_magnet_current_check(void);

/**
 * @brief Initialize the magnets.
 * @return 0 on success, non-zero otherwise
 */
int ctrl_magnet_init(void);

/**
 * @brief Reset magnets to defaults.
 * @return 0 on success, non-zero otherwise
 */
int ctrl_magnet_reinit(void);

/**
 * @brief Initialize the magnet error handler.
 */
void ctrl_magnet_error_handler_init(void);



#ifdef __cplusplus
}
#endif

#endif /* CONTROL_MAGNET_H */
