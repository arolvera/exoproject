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
/* ************************************************************************** */
/** Private include detailing the flash memory layout 

  @Company
    Exoterra

  @File Name
    storage_memory_layout.h

  @Summary
    Private include detailing the flash memory layout for the SAMV71 update process

  @Description
    Define common macros to be used for organizing device images in flash 
 */
/* ************************************************************************** */

#ifndef _MEMORY_LAYOUT_H   /* Guard against multiple inclusion */
#define _MEMORY_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "device.h"

#define RR_NUM_COPIES 3

#define STORAGE_LAYOUT_V1   1
#define STORAGE_LAYOUT_V2   2

#ifndef STORAGE_LAYOUT 
#define STORAGE_LAYOUT      STORAGE_LAYOUT_V2
#endif
    
#define FLASH_START_ADDRESS                 0x400000
#define FLASH_END_ADDRESS                   0x5FFFFF
#define BOOT_LOADER                         FLASH_START_ADDRESS
#define APPLICATION                         0x410000
#define MASTER_IMAGE_FLASH_START            0x440000
    
#if  STORAGE_LAYOUT == STORAGE_LAYOUT_V1
/* Sector 4 */  
#define KEEPER_EXECUTION_HEADER_1           0x480000
#define ANODE_EXECUTION_HEADER_1            0x482000
#define VALVES_EXECUTION_HEADER_1           0x484000
#define APP_RECOVERY_EXECUTION_HEADER_1     0x486000
#define OUTER_MAGNET_EXECUTION_HEADER_1     0x488000
#define SYSTEM_CONTROL_EXECUTION_HEADER_1   0x48A000
#define BOOTLOADER_EXECUTION_HEADER_1       0x48C000
#define KEEPER_IMAGE_COPY_1                 0x48E000
#define ANODE_IMAGE_COPY_1                  0x496000
#define SECTOR_4_DEAD_SPACE                 0x49E000
/* Sector 5 */
#define VALVES_IMAGE_COPY_1                 0x4A0000
#define APP_RECOVERY_IMAGE_COPY_1           0x4A8000
#define OUTER_MAGNET_IMAGE_COPY_1           0x4B0000
#define BOOTLOADER_IMAGE_COPY_1             0x4B8000
/* Sector 6 */
#define SYSTEM_CONTROL_IMAGE_COPY_1         0x4C0000
/* Sector 7 */
#define KEEPER_EXECUTION_HEADER_2           0x4E0000
#define ANODE_EXECUTION_HEADER_2            0x4E2000
#define VALVES_EXECUTION_HEADER_2           0x4E4000
#define APP_RECOVERY_EXECUTION_HEADER_2     0x4E6000
#define OUTER_MAGNET_EXECUTION_HEADER_2     0x4E8000
#define SYSTEM_CONTROL_EXECUTION_HEADER_2   0x4EA000
#define BOOTLOADER_EXECUTION_HEADER_2       0x4EC000
#define KEEPER_IMAGE_COPY_2                 0x4EE000
#define ANODE_IMAGE_COPY_2                  0x4F6000
#define SECTOR_7_DEAD_SPACE                 0x4FE000
/* Sector 8 */
#define VALVES_IMAGE_COPY_2                 0x500000
#define APP_RECOVERY_IMAGE_COPY_2           0x508000
#define OUTER_MAGNET_IMAGE_COPY_2           0x510000
#define BOOTLOADER_IMAGE_COPY_2             0x518000
/* Sector 9 */
#define SYSTEM_CONTROL_IMAGE_COPY_2         0x520000
/* Sector 10 */
#define KEEPER_EXECUTION_HEADER_3           0x540000
#define ANODE_EXECUTION_HEADER_3            0x542000
#define VALVES_EXECUTION_HEADER_3           0x544000
#define APP_RECOVERY_EXECUTION_HEADER_3     0x546000
#define OUTER_MAGNET_EXECUTION_HEADER_3     0x548000
#define SYSTEM_CONTROL_EXECUTION_HEADER_3   0x54A000
#define BOOTLOADER_EXECUTION_HEADER_3       0x54C000
#define KEEPER_IMAGE_COPY_3                 0x54E000
#define ANODE_IMAGE_COPY_3                  0x556000
#define SECTOR_10_DEAD_SPACE                0x55E000
/* Sector 11 */
#define VALVES_IMAGE_COPY_3                 0x560000
#define APP_RECOVERY_IMAGE_COPY_3           0x568000
#define OUTER_MAGNET_IMAGE_COPY_3           0x570000
#define BOOTLOADER_IMAGE_COPY_3             0x578000
/* Sector 12 */
#define SYSTEM_CONTROL_IMAGE_COPY_3         0x580000
/* Sector 13 */
#define CAN_OPEN_AREA                       0x5A0000
#define CAN_OPEN_SIZE                       (IFLASH_PAGE_SIZE * 16)
    
#define EXECUTION_HEADER_SIZE               (IFLASH_PAGE_SIZE * 16)
#define MASTER_IMAGE_AREA                   0x40000  /* 2 sectors             */
#define BOOTLOADER_AREA                     0x08000  /* Max Bootload size     */
#define SYSTEM_CONTROL_COPY_AREA            0x20000  /* MAX App Size 1 Sector */
#define IMAGE_COPY_AREA                     0x08000  /* MCU MAX SIZE          */
#define NUM_IMAGE_COPIES                    3

#elif  STORAGE_LAYOUT == STORAGE_LAYOUT_V2
#define KEEPER_EXECUTION_HEADER_1           0x480000
#define ANODE_EXECUTION_HEADER_1            0x480200
#define VALVES_EXECUTION_HEADER_1           0x480400
#define MAGNET_EXECUTION_HEADER_1           0x480600
#define SYSTEM_CONTROL_EXECUTION_HEADER_1   0x480800
#define BOOTLOADER_EXECUTION_HEADER_1       0x480A00
#define RESERVED_1                          0x480C00
#define KEEPER_IMAGE_COPY_1                 0x482000
#define ANODE_IMAGE_COPY_1                  0x48A000
#define MAGNET_IMAGE_COPY_1                 0x492000
#define VALVES_IMAGE_COPY_1                 0x49A000
 /* Sector 5 */                      
#define VALVES_IMAGE_COPY_1_2               0x4A0000
#define BOOTLOADER_IMAGE_COPY_1             0x4A2000
#define SYSTEM_CONTROL_IMAGE_COPY_1         0x4AA000
 /* Sector 6 */                      
#define SYSTEM_CONTROL_IMAGE_COPY_1_2       0x4C0000
 /* Sector 7 */                      
#define KEEPER_EXECUTION_HEADER_2           0x4E0000
#define ANODE_EXECUTION_HEADER_2            0x4E0200
#define VALVES_EXECUTION_HEADER_2           0x4E0400
#define MAGNET_EXECUTION_HEADER_2           0x4E0600
#define SYSTEM_CONTROL_EXECUTION_HEADER_2   0x4E0800
#define BOOTLOADER_EXECUTION_HEADER_2       0x4E0A00
#define RESERVED_2                          0x4E0C00
#define KEEPER_IMAGE_COPY_2                 0x4E2000
#define ANODE_IMAGE_COPY_2                  0x4EA000
#define MAGNET_IMAGE_COPY_2                 0x4F2000
#define VALVES_IMAGE_COPY_2                 0x4FA000
 /* Sector 8 */                      
#define VALVES_IMAGE_COPY_2_2               0x500000
#define BOOTLOADER_IMAGE_COPY_2             0x502000
#define SYSTEM_CONTROL_IMAGE_COPY_2         0x50A000
 /* Sector 9 */                      
#define SYSTEM_CONTROL_IMAGE_COPY_2_2       0x520000
 /* Sector 10 */                     
#define KEEPER_EXECUTION_HEADER_3           0x540000
#define ANODE_EXECUTION_HEADER_3            0x540200
#define VALVES_EXECUTION_HEADER_3           0x540400
#define MAGNET_EXECUTION_HEADER_3           0x540600
#define SYSTEM_CONTROL_EXECUTION_HEADER_3   0x540800
#define BOOTLOADER_EXECUTION_HEADER_3       0x540A00
#define RESERVED_3                          0x540C00
#define KEEPER_IMAGE_COPY_3                 0x542000
#define ANODE_IMAGE_COPY_3                  0x54A000
#define MAGNET_IMAGE_COPY_3                 0x552000
#define VALVES_IMAGE_COPY_3                 0x55A000
 /* Sector 11 */                     
#define VALVES_IMAGE_COPY_3_2               0x560000
#define BOOTLOADER_IMAGE_COPY_3             0x562000
#define SYSTEM_CONTROL_IMAGE_COPY_3         0x56A000
 /* Sector 12 */                     
#define SYSTEM_CONTROL_IMAGE_COPY_3_2       0x580000
 /* Sector 13 */                     
#define CLIENT_LOCKOUT_TIMER                0x5A0000
#define UNUSED_SECTOR_13                    0x5A0200
 /* Sector 14 */                     
#define UPDATER_TRIGGER_AREA                0x5C0000
#define CONDITION_STAT_AREA                 0x5DFE00
    /* Sector 15 */
#define UNUSED_SECTOR_15                    0x5E0000
#define USER_CONFIG_VAR_EXECUTION_HEADER_1  0x5E1A00
#define USER_CONFIG_VAR_EXECUTION_HEADER_2  0x5E1C00
#define USER_CONFIG_VAR_EXECUTION_HEADER_3  0x5E1E00
#define USER_CONFIG_VAR_IMAGE_COPY_1        0x5E2000
#define USER_CONFIG_VAR_IMAGE_COPY_2        0x5EC000
#define USER_CONFIG_VAR_IMAGE_COPY_3        0x5F6000
#define USER_CONFIG_VAR_IMAGE_COPY_SIZE     0x00A000


#define EXECUTION_HEADER_SIZE               (IFLASH_PAGE_SIZE)
#define MASTER_IMAGE_AREA                   0x40000  /* 2 sectors             */
#define BOOTLOADER_AREA                     0x08000  /* Max Bootload size     */
#define SYSTEM_CONTROL_COPY_AREA            0x36000
#define IMAGE_COPY_AREA                     0x08000  /* MCU MAX SIZE          */
#define CONDITION_STAT_SIZE                 0x00200
#define LOCKOUT_NVM_SIZE                    0x00200

#endif

#ifdef __cplusplus
}
#endif
    
#endif /* _MEMORY_LAYOUT_H */

/* *****************************************************************************
 End of File
 */
