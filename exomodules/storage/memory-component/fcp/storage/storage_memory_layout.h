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
#include "samv71q21b.h"

#define RR_NUM_COPIES 3

#define FLASH_START_ADDRESS                 0x400000
#define FLASH_END_ADDRESS                   0x5FFFFF
#define BOOT_LOADER                         FLASH_START_ADDRESS
#define APPLICATION                         0x410000
#define MASTER_IMAGE_FLASH_START            0x440000

/* Sector 4 */
#define BUCK_EXECUTION_HEADER_1             0x480000
#define FLIGHT_CONTROL_EXECUTION_HEADER_1   0x48A000
#define BOOTLOADER_EXECUTION_HEADER_1       0x48C000
#define BUCK_IMAGE_COPY_1                   0x48E000
#define BOOTLOADER_IMAGE_COPY_1             0x4B8000
/* Sector 6 */
#define FLIGHT_CONTROL_IMAGE_COPY_1         0x4C0000
/* Sector 7 */
#define BUCK_EXECUTION_HEADER_2             0x4E0000
#define FLIGHT_CONTROL_EXECUTION_HEADER_2   0x4EA000
#define BOOTLOADER_EXECUTION_HEADER_2       0x4EC000
#define BUCK_IMAGE_COPY_2                   0x4EE000
#define BOOTLOADER_IMAGE_COPY_2             0x518000
/* Sector 9 */
#define FLIGHT_CONTROL_IMAGE_COPY_2         0x520000
/* Sector 10 */
#define BUCK_EXECUTION_HEADER_3             0x540000
#define FLIGHT_CONTROL_EXECUTION_HEADER_3   0x54A000
#define BOOTLOADER_EXECUTION_HEADER_3       0x54C000
#define BUCK_IMAGE_COPY_3                   0x54E000
#define BOOTLOADER_IMAGE_COPY_3             0x578000
/* Sector 12 */
#define FLIGHT_CONTROL_IMAGE_COPY_3         0x580000
/* Sector 13 */
#define CAN_OPEN_AREA                       0x5A0000
#define CAN_OPEN_SIZE                       (IFLASH_PAGE_SIZE * 16)

#define UPDATER_TRIGGER_AREA                0x5C0000

#define EXECUTION_HEADER_SIZE               (IFLASH_PAGE_SIZE * 16)
#define MASTER_IMAGE_AREA                   0x40000  /* 2 sectors             */
#define BOOTLOADER_AREA                     0x08000  /* Max Bootload size     */
#define FLIGHT_CONTROL_COPY_AREA            0x20000  /* MAX App Size 1 Sector */
#define IMAGE_COPY_AREA                     0x08000  /* MCU MAX SIZE          */
#define NUM_IMAGE_COPIES                    3



#ifdef __cplusplus
}
#endif

#endif /* _MEMORY_LAYOUT_H */

/* *****************************************************************************
End of File
*/
