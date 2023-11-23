/**
 * @file    command_magnets.h
 *
 * @brief   Interface for magnet commands.
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

#ifndef COMMAND_MAGNETS_H
#define COMMAND_MAGNETS_H

#ifdef	__cplusplus
extern "C" {
#endif



/**
 * @brief Process command to get the current on/off state of the magnets.
 * @return State value
 */
uint8_t cmd_magnet_state_get(void);

/**
 * @brief Process command to turn magnets on/off.
 * @param state COMMANDED_ON or COMMANDED_OFF
 * @return 0 on success, non-zero otherwise
 */
int cmd_magnet_state_set(uint8_t state);

/**
 * @brief Process a command to read the firmware version of the magnet controller
 * @param version Pointer to a firmware version object to write the result to.
 * @return 0 on success, non-zero otherwise
 */
int cmd_magnets_version_get(uint32_t** version);

/**
 * @brief Process a command to set the current.
 * @param current Current value to set the Outer Magnet to
 * @return 0 on success, non-zero otherwise
 */
int cmd_magnet_current_set(float current);

/**
 * @brief Process a command to read the magnet current setting.
 * @return Ampere version of current
 */
float cmd_magnet_current_get();

/**
 * @brief Process a command to read the factored magnet current setting.
 * @return Counts version of current
 */
uint16_t cmd_magnet_current_factored_get();

/**
 * @brief Process a command to check that the magnet current is in an acceptable range according
 * to the current set point.
 * @return 0 if in range, non-zero otherwise
 */
int cmd_magnet_current_check(void);

/**
 * @brief Process a command to initialize the magnets.
 * @return 0 on success, non-zero otherwise
 */
int cmd_magnet_init(void);



#ifdef	__cplusplus
}
#endif

#endif /* COMMAND_MAGNETS_H */
