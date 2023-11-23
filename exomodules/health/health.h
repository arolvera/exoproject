/**
 * @file    health.h
 *
 * @brief   ??? Describe this module
 *
 * What is there left to do? We have some unused functions.
 * What is the difference between this and the client_health module?
 * Why do we have an empty gitignore in the include folder?
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

#ifndef HEALTH_STATUS_H
#define	HEALTH_STATUS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "mcu_include.h"
#include "thruster_control.h"



typedef void (*hsi_conv_func_t)(uint16_t);
void health_init();
void health_enable(bool enable);
bool health_enabled(void);
bool health_valid(void);

void health_mcu_register(component_type_t dev_id, health_table_entry_t** ph, hsi_conv_func_t cf );
void health_mcu_disable(component_type_t dev_id);

int health_device_hsi_recieved(component_type_t dev_id);
uint32_t health_device_misses_get(void);



#ifdef	__cplusplus
}
#endif

#endif	/* HEALTH_STATUS_H */

