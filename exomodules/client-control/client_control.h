/**
 * @file    client_control.h
 *
 * @brief   Utilities for controlling client processors.
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

#ifndef CLIENT_CONTROL_H
#define CLIENT_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sequence/control_sequence.h"
#include "client_service.h"
#include "client_booted.h"
#include "stdbool.h"
#include "thruster_control.h"



/**
 * @brief
 */
void client_init();

/**
 * @brief
 */
void client_update_state();

/**
 * @brief
 */
void client_emcy_shutdown(void);

/**
 * @brief Read all component firmware version and Git SHA values from the update
 * header stored in persistent memory (NOR Flash for Halo 12).
 * @param firmware_versions
 */
void client_internal_version_OD_ptrs_populate(uint32_t* (*firmware_versions)[2]);

/**
 * @brief
 */
void client_control_error_handler_init(void);

/**
 * @brief
 * @return
 */
size_t client_error_detail_size_get(void);

/**
 * @brief
 * @param abort_err
 * @return
 */
int client_error_check(sequence_abort_error_t abort_err);

/**
 * @brief
 */
void client_steady_state_err_clr(void);

/**
 * @brief
 * @return
 */
bool client_get_conditioning_task_state(void);

/**
 * @brief
 * @param is_conditioning
 */
void client_set_conditioning_task_state(bool is_conditioning);

/**
 * @brief
 */
void client_lockout_override(void);



#ifdef __cplusplus
}
#endif

#endif /* CLIENT_CONTROL_H */
