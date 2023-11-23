
/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

Unauthorized copying of this file, via any medium is strictly prohibited
                                       Proprietary and confidential.  Any unauthorized use, duplication, transmission,
    distribution, or disclosure of this software is expressly forbidden.

                     This Copyright notice may not be removed or modified without prior written
           consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

       ExoTerra Corp
       7640 S. Alkire Pl.
       Littleton, CO 80127
    USA

        Voice:  +1 1 (720) 788-2010
                   http:   www.exoterracorp.com
                         email:  contact@exoterracorp.com
                                    */
#include <string.h>
#include <sys/stat.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "utils/macro_tools.h"
#include "storage/storage_class.h"
#include "storage/storage_memory_layout.h"
#include "storage/component_keys.h"


/* These must remain in KEY order - Alphabetical from A-Z then a-z */
component_map_t file_map[] = {
    {UPDATE_IMAGE,           {MASTER_IMAGE_FLASH_START, MASTER_IMAGE_FLASH_START + MASTER_IMAGE_AREA}, false,                          sm_erase_write},
    {UNUSED_0, {-1,  0}, false,                                                                                                        sm_erase_write},
    
    {BUCK_EXEC_HDR_1, {BUCK_EXECUTION_HEADER_1, BUCK_EXECUTION_HEADER_1 + EXECUTION_HEADER_SIZE}, false,                               sm_overlay_write},
    {BUCK_EXEC_HDR_2, {BUCK_EXECUTION_HEADER_2, BUCK_EXECUTION_HEADER_2 + EXECUTION_HEADER_SIZE}, false,                               sm_overlay_write},
    {BUCK_EXEC_HDR_3, {BUCK_EXECUTION_HEADER_3, BUCK_EXECUTION_HEADER_3 + EXECUTION_HEADER_SIZE}, false,                               sm_overlay_write},
    
    {BUCK_1, {BUCK_IMAGE_COPY_1, BUCK_IMAGE_COPY_1 + IMAGE_COPY_AREA}, false,                                                          sm_erase_write},
    {BUCK_2, {BUCK_IMAGE_COPY_2, BUCK_IMAGE_COPY_2 + IMAGE_COPY_AREA}, false,                                                          sm_erase_write},
    {BUCK_3, {BUCK_IMAGE_COPY_3, BUCK_IMAGE_COPY_3 + IMAGE_COPY_AREA}, false,                                                          sm_erase_write},
    
    {FLIGHT_CONTROL_EXEC_HDR_1, {FLIGHT_CONTROL_EXECUTION_HEADER_1, FLIGHT_CONTROL_EXECUTION_HEADER_1 + EXECUTION_HEADER_SIZE}, false, sm_overlay_write},
    {FLIGHT_CONTROL_EXEC_HDR_2, {FLIGHT_CONTROL_EXECUTION_HEADER_2, FLIGHT_CONTROL_EXECUTION_HEADER_2 + EXECUTION_HEADER_SIZE}, false, sm_overlay_write},
    {FLIGHT_CONTROL_EXEC_HDR_3, {FLIGHT_CONTROL_EXECUTION_HEADER_3, FLIGHT_CONTROL_EXECUTION_HEADER_3 + EXECUTION_HEADER_SIZE}, false, sm_overlay_write},
    
    {FLIGHT_CONTROL_1, {FLIGHT_CONTROL_IMAGE_COPY_1, FLIGHT_CONTROL_IMAGE_COPY_1 + FLIGHT_CONTROL_COPY_AREA}, false,                   sm_erase_write},
    {FLIGHT_CONTROL_2, {FLIGHT_CONTROL_IMAGE_COPY_2, FLIGHT_CONTROL_IMAGE_COPY_2 + FLIGHT_CONTROL_COPY_AREA}, false,                   sm_erase_write},
    {FLIGHT_CONTROL_3, {FLIGHT_CONTROL_IMAGE_COPY_3, FLIGHT_CONTROL_IMAGE_COPY_3 + FLIGHT_CONTROL_COPY_AREA}, false,                   sm_erase_write},
 
    
    {BOOTLOADER_EXEC_HDR_1, {BOOTLOADER_EXECUTION_HEADER_1, BOOTLOADER_EXECUTION_HEADER_1 + EXECUTION_HEADER_SIZE}, false,             sm_overlay_write},
    {BOOTLOADER_EXEC_HDR_2, {BOOTLOADER_EXECUTION_HEADER_2, BOOTLOADER_EXECUTION_HEADER_2 + EXECUTION_HEADER_SIZE}, false,             sm_overlay_write},
    {BOOTLOADER_EXEC_HDR_3, {BOOTLOADER_EXECUTION_HEADER_3, BOOTLOADER_EXECUTION_HEADER_3 + EXECUTION_HEADER_SIZE}, false,             sm_overlay_write},
    
    {BOOTLOADER_1, {BOOTLOADER_IMAGE_COPY_1, BOOTLOADER_IMAGE_COPY_1 + BOOTLOADER_AREA}, false,                                        sm_erase_write},
    {BOOTLOADER_2, {BOOTLOADER_IMAGE_COPY_2, BOOTLOADER_IMAGE_COPY_2 + BOOTLOADER_AREA}, false,                                        sm_erase_write},
    {BOOTLOADER_3, {BOOTLOADER_IMAGE_COPY_3, BOOTLOADER_IMAGE_COPY_3 + BOOTLOADER_AREA}, false,                                        sm_erase_write},
    
    {APPLICATION_EXE, {APPLICATION, APPLICATION + FLIGHT_CONTROL_COPY_AREA}, false,                                                    sm_erase_write},
    
    {TC_BOOTLOADER, {BOOT_LOADER, BOOT_LOADER + BOOTLOADER_AREA}, false,                                                               sm_erase_write},

    {UPDATER_TRIGGER, {UPDATER_TRIGGER_AREA, UPDATER_TRIGGER_AREA + IFLASH_PAGE_SIZE}, false,                                          sm_overlay_write}

};

const size_t FILE_MAP_SIZE = (sizeof(file_map)/sizeof(file_map[0]));
