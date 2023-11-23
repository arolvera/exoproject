/**
 * @file    client_power.h
 *
 * @brief   ??? Control power to the anode, keeper, magnets and valves. Because
 * the keeper is operated on the ECPK, controlling power simply means starting
 * or stopping a task for the keeper. Contolling power to the ACP and MVCP means
 * turning the power supply to the Vorago chip on or off.
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

#ifndef CLIENT_POWER_H
#define CLIENT_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "thruster_control.h"

/**
 * @brief Set or clear power for each component; keeper, anode, magnets and valves.
 * In the case of the keeper, this will either start the keeper task or destroy it.
 * In the case of anode, magnets, and valves, this will power on or off the vorago
 * processors controlling those components.
 * @param power_on True = power on or start, False = power down or stop.
 */
void cp_power_set(bool power_on);

/**
 * @brief reset all the clients
 */
void cp_reset_all(void);

/**
 * @brief reset Individual client
 */
int cp_reset_individual(int client_id);

#ifdef __cplusplus
}
#endif

#endif /* CLIENT_POWER_H */
