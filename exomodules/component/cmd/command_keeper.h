/**
 * @file    command_keeper.h
 *
 * @brief   Interface for keeper commands common to both Halo 6 and 12.
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

#ifndef COMMAND_KEEPER_H
#define	KEEPER_COMMAND_H
#include <math.h>
#include "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif



/**
 * @brief Process command to get the keeper power supply state.
 * @return COMMANDED_ON or COMMANDED_OFF
 */
uint8_t cmd_keeper_ps_state_get(void);

/**
 * @brief Process command to set the state of the keeper power supply
 * @param on COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_ps_state_set(uint8_t on);

/**
 * @brief Process a command to get the Keeper state.
 * @return keeper state
 */
uint16_t cmd_keeper_state_get(void);

/**
 * @brief Process a command to get the Keeper State Status. State status is
 * the reason it got into the state it is in. For example, the state could be
 * 0xff which is an error. The State Status might then be under-voltage or
 * over-current.
 * @return keeper state status
 */
uint16_t cmd_keeper_state_stat_get(void);

/**
 * @brief Process command to get keeper current value. This is the value that is SET.
 * The actual current value comes back from the client in the HSI message.
 * @return Keeper current
 */
float_t cmd_keeper_cur_get(void);

/**
 * @brief Process command to get the factored current in counts.
 * @return Factored keeper current
 */
uint16_t cmd_keeper_cur_factored_get(void);

/**
 * @brief Process command to set the keeper current.
 * @param current Current in amps
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_cur_set(float_t current);

/**
 * @brief @brief Process command to get anode voltage value. This is the value that is SET.
 * The actual voltage value comes back from the client in the HSI message.
 * @return
 */
float_t cmd_keeper_volts_get(void);

/**
 * @brief Process command to get the factored voltage in counts.
 * @return
 */
uint16_t cmd_keeper_volts_factored_get(void);

/**
 * @brief Process command to set the keeper voltage.
 * @param voltage Voltage in volts
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_volts_set(float_t voltage);

/**
 * @brief Process a command to start the keeper and wait for a spark.
 * @param timeout How long to wait for spark
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_start(uint32_t timeout);

/**
 * @brief Process a command to reset the keeper.
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_reinit(void);

/**
 * @brief Process a command to initialize the keeper.
 * @return 0 on success, non-zero otherwise
 */
int cmd_keeper_init(void);



#ifdef	__cplusplus
}
#endif

#endif	/* KEEPER_COMMAND_H */

