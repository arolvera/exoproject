/**
 * @file    storage_helper.h
 *
 * @brief   Helper functions for accessing 'things' with multiple redundant
 * copies in memory.
 *
 * @Description When there are multiple redundant files in memory, there must
 * be decisions made about which copy is active and which copy to use.  Also,
 * some data files have information stored about their contents in separate areas
 * to make reading the data smoother (no worries about where the header starts
 * and ends). This module makes that translation and does all the decision making.
 * This takes the burden off the callers and let them focus on what they should be
 * doing (bootloading the atmegas, and not worrying about active data file copies,
 * for example).
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

#ifndef STORAGE_HELPER_H
#define STORAGE_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "master_image_constructor.h"
#include "component_type.h"

typedef struct stat_buf{
    struct stat buf;
    uint16_t crc;
    uint32_t version;
    uint8_t region;
} stat_helper_t;

/**
 * @brief Read the image offset for the selected device in the update header.
 * @param device Number assigned to the ECPK, MVCP, or ACP
 * @param mem_opt Where to read the update header from; 0 = SRAM, 1 = active NOR flash region
 * @param offset Offset for the device image starting from the active backup location
 * @return 0 = success, non-zero otherwise
 */
int sh_get_device_image_offset(device_type_t device, uint8_t mem_opt, uint32_t* offset);

/**
 * @brief Read the update header associated with an installed firmware image
 * stored in the 'active' persistent storage area. Extract the firmware version
 * and Git SHA.
 * @param device Number assigned to the ECPK, MVCP, or ACP
 * @param version Output variable to receive the concatenated major.minor.version
 * @param git_sha Output variable to receive the Git SHA value
 * @return 0 = success, non-zero otherwise
 */
int sh_get_active_versions(device_type_t device, uint32_t* version, uint32_t* git_sha);

/**
 * @brief There are two backup regions in storage; one active and the other stale.
 * @return Backup region number that is currently active (installed in FRAM), 1 or 2
 */
uint8_t sh_get_software_active_backup_region(void);

/**
 * @brief There are two backup regions in storage; one active and the other stale.
 * This function swaps the status for these two regions.
 */
void sh_swap_software_backup_region(void);


int sh_init(void);

int sh_app_stat_get(volatile app_stat_t* app_stat);

int sh_app_stat_set(volatile app_stat_t* app_stat);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_HELPER_H */
