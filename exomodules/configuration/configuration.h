/**
 * @file    storage_memory_interface.h
 *
 * @brief   This file contains the data structure definitions for application
 * configuration data stored in persistent memory.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>



typedef struct {
    bool                    update_region1_is_stale;
    bool                    update_region2_is_stale;
    uint8_t                 padding[2];                     // just trying for 32-bit boundaries
} update_config_data_t;

typedef struct {
    update_config_data_t    update_config_data;
} app_configuration_t;



#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_H */