/**
 * @file    client_p.h
 *
 * @brief   ??? Private include file for Client control. Data structures and functions
 * are intended only to be used within the client control module.
 *
 * Appears to only need a refactor of CLIENT_CONTROL_SUBMODULES. Anything else to do?
 * Can we delete the unused defines? Yes.
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

#ifndef CLIENT_PRIVATE_H
#define CLIENT_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can/hal_can.h"  // For can DLC type
#include "error/fault_handler.h"



#define CAN_ERR_CAN_ERR      1
#define CAN_ERR_HEALTH_INDEX 2
#define SIZEOF_VERSION sizeof(uint32_t)
#define SIZEOF_GIT_SHA sizeof(uint32_t)

#define SEQUENCE_MS_DELAY(__X__) vTaskDelay(__X__/portTICK_RATE_MS)
#define CLIENT_MSG_TIMEOUT_DEFAULT (500/portTICK_RATE_MS)

/* ToDo: Move SUBMODULE to before unique word */
typedef enum {
    CLIENT_CONTROL_SUBMODULE,
    CLIENT_CONTROL_POWER_SUBMODULE,
    CLIENT_CONTROL_ANODE_SUBMODULE,
    CLIENT_CONTROL_CONDITION_SUBMODULE,
    CLIENT_CONTROL_KEEPER_SUBMODULE,
    CLIENT_CONTROL_MAGNETS_SUBMODULE,
    CLIENT_CONTROL_SEQUENCE_SUBMODULE,
    CLIENT_CONTROL_THROTTLE_SUBMODULE,
    CLIENT_CONTROL_THRUST_SUBMODULE,
    CLIENT_CONTROL_THRUSTER_START_SUBMODULE,
    CLIENT_CONTROL_VALVES_SUBMODULE,
    CLIENT_CONTROL_LOCKOUT,
    CLIENT_CONTROL_HEALTH,
    CLIENT_CONTROL_BOOTED,
    CLIENT_CONTROL_SUBMODULE_EOL,
} CLIENT_CONTROL_SUBMODULES;

typedef struct control_state_t_ {
    uint8_t state;
    uint8_t reason;
} control_state_t;

typedef struct client_msg {
    uint16_t cobid;
    uint8_t *data;
    can_dlc_t dlc;
    uint32_t timeout;
    SemaphoreHandle_t *psem;
} client_msg_t;

typedef struct client_bcast_error_stat {
    unsigned int error_code;
    unsigned int error_type;
    fault_handlers_t fault_handler;
} client_bcast_error_stat_t;

typedef struct {
    uint32_t git_sha;
    uint32_t version;
} exec_app_versions_t;

typedef struct{
    uint8_t rev; // Reserved byte
    uint8_t minor;
    uint8_t major;
    uint8_t reserved;
    uint32_t git_sha;
} version_info_t;



/* Specialization event call backs into common function */
typedef void (*running_evt_cb_t) (void);
typedef void (*error_evt_cb_t) (uint32_t);
typedef struct _event_callback_t {
    running_evt_cb_t running;
    error_evt_cb_t error;
} event_callback_t;

int client_cmd_send(client_msg_t *msg);



#ifdef __cplusplus
}
#endif

#endif /* CLIENT_PRIVATE_H */
