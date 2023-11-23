/**
 * @file    command_anode.h
 *
 * @brief   Interface for anode commands common to both Halo 6 and 12.
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

#ifndef COMMAND_ANODE_H
#define	COMMAND_ANODE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "math.h"



/**
 * @brief Process command to get the anode power supply state.
 * @return COMMANDED_ON or COMMANDED_OFF
 */
uint8_t cmd_anode_ps_state_get(void);

/**
 * @brief Process command to set the state of the anode power supply
 * @param on COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int cmd_anode_ps_state_set(uint8_t on);

/**
 * @brief Process command to get anode current value. This is the value that is SET.
 * The actual current value comes back from the client in the HSI message.
 * @return Anode current
 */
float_t cmd_anode_cur_get(void);

/**
 * @brief Process command to get the factored current in counts.
 * @return Factored anode current
 */
uint16_t cmd_anode_cur_factored_get(void);

/**
 * @brief Process command to set the anode current.
 * @param current Current in amps
 * @return 0 on success, non-zero otherwise
 */
int cmd_anode_cur_set(float_t current);

/**
 * @brief @brief Process command to get anode voltage value. This is the value that is SET.
 * The actual voltage value comes back from the client in the HSI message.
 * @return
 */
float_t cmd_anode_volts_get(void);

/**
 * @brief Process command to get the factored voltage in counts.
 * @return
 */
uint16_t cmd_anode_volts_factored_get(void);

/**
 * @brief Process command to set the anode voltage.
 * @param voltage Voltage in volts
 * @return 0 on success, non-zero otherwise
 */
int cmd_anode_volts_set(float_t voltage);

/**
 * @brief Process command to monitor the stability of the anode current.
 * @param target_current Expected current value
 * @return 0 = stable, non-zero otherwise
 */
int cmd_anode_monitor_stability(float_t target_current);

/**
 * @brief Process a command to reset the anode.
 * @return 0 on success, non-zero otherwise
 */
int cmd_anode_reinit(void);

/**
 * @brief Process command to initialize the anode.
 * @return 0 on success, non-zero otherwise
 */
int cmd_anode_init(void);



#ifdef	__cplusplus
}
#endif

#endif	/* COMMAND_ANODE_H */

