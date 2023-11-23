/**
 * @file    client_booted.h
 *
 * @brief   ??? Utility functions to monitor and manage client bootup.
 *
 * client_health_bootup_check() is used by client_update_state() but doesn't do anything. What should we do? Get rid of it
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

#ifndef CLIENT_BOOTED_H
#define CLIENT_BOOTED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "component_type.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    CBS_POWERED_OFF,
    CBS_BOOTING,
    CBS_BOOTED,
} client_boot_state_t;

/* Global structure to hold current boot status */
typedef struct client_boot_status {
    uint8_t expected;
    uint8_t booted;
    client_boot_state_t state;
} client_boot_status_t;

typedef enum{
    BS_BOOT_ERROR_GENERAL,
    BS_MISSING_DEVICE_BOOT,
    BS_BOOT_TIMEOUT,
    BS_BOOT_ATTEMPTS_EXCEEDED,
    BS_HSI_MISSES_EXCEEDED,
    BS_HSI_UNEXPECTED_BOOT,
    BS_EOL,
} boot_status_t;


void client_control_register_component(component_type_t c);
void client_comp_powered_down(component_type_t c, bool resetting);
void client_booted_register_component(component_type_t c);
void client_boot_err(boot_status_t boot_err);
int client_boot_timout_check(void);
int client_health_bootup_check(void);
void client_power_up_timer(void);
void client_power_down_timer(void);
uint16_t client_boot_status_get_expected(void);
uint16_t client_boot_status_get_booted(void);
client_boot_state_t client_boot_status_get_state(void);


#ifdef __cplusplus
}
#endif

#endif //CLIENT_BOOTED_H
