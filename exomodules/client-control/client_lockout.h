/**
 * @file    client_lockout.h
 *
 * @brief   ??? What is client lockout?
 *
 * Many functions are commented out, associated with iacm. What needs to be done?
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

#ifndef CLIENT_LOCKOUT_H
#define CLIENT_LOCKOUT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "iacm/iacm.h"



typedef enum{
    LOCKOUT_ERROR_SET,
    LOCKOUT_ERROR_CLEAR,
}lockout_action_t;

/**
 * @brief
 */
void client_lockout_init(void);

/**
 * @brief
 */
void client_lockout_expired(void);

/**
 * @brief
 */
void client_lockout_override(void);

/**
 * @brief
 * @param reason
 */
void client_lockout_queue(int reason);

/**
 * @brief
 * @param lockout_action
 */
void client_lockout_status_set(lockout_action_t lockout_action);

/**
 * @brief
 * @param reason
 */
void client_lockout_reason_set(uint32_t reason);

/**
 * @brief
 * @param iacm_lockout_state
 */
void client_lockout_val_set(iacm_thruster_state_t iacm_lockout_state);

/**
 * @brief
 * @return
 */
uint32_t client_lockout_timer_get(void);

/**
 * @brief
 * @return
 */
uint32_t client_lockout_val_get(void);

/**
 * @brief
 */
void client_lockout_update_timer(void);



#ifdef	__cplusplus
}
#endif

#endif //CLIENT_LOCKOUT_H
