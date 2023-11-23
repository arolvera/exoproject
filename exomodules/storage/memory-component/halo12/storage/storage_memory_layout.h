/**
 * @file    storage_memory_layout.h
 *
 * @brief   Memory address definitions for the Halo 12 memory layout.
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

#ifndef STORAGE_MEMORY_LAYOUT_H
#define STORAGE_MEMORY_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif


#define APP_VAR_SPACE                       0x13000
#define FRAM_START_ADDR                     0x00000000
#define SRAM_APP_DATA_START_ADDR            0x60000000
//#define SRAM_UPDATE_START_ADDR              0x60080000      // uncomment when running on PPU board
//#define EBI_NOR_START_ADDR                  0x60100000      // uncomment when running on PPU board
#define SRAM_UPDATE_START_ADDR              (0x60000000 + APP_VAR_SPACE)      // uncomment when running on dev board
#define EBI_NOR_START_ADDR                  0x60040000      // uncomment when running on dev board

#define APP_CONFIG_SIZE                     0x100
#define HEADER_REGION_SIZE                  0x100
#define BIN_IMAGE_MAX_SIZE                  0x40000 //We boot from FRAM cannot be bigger than this

/* 1MB reserved in SRAM for temporary storage of update images as well as the two
 * backup regions of NOR flash. This allows future deployment of separate images
 * for each device, rather than one image for them all as we do now. */
//#define UPDATE_REGION_SIZE                  0x80000      // uncomment when running on PPU board
#define UPDATE_REGION_SIZE                  0x2d000      // uncomment when running on PPU board

#define APPLICATION_FLASH_START             FRAM_START_ADDR
#define NOR_UPDATE_BKUP1_START_ADDR         EBI_NOR_START_ADDR
/* add 0x14000 to end on sector boundary for flash erase/write operations */
#define NOR_UPDATE_BKUP2_START_ADDR         (EBI_NOR_START_ADDR + UPDATE_REGION_SIZE + APP_VAR_SPACE)

#define NOR_APP_CONFIGURATION_ADDR          (NOR_UPDATE_BKUP2_START_ADDR + UPDATE_REGION_SIZE + APP_VAR_SPACE)

/* Setting aside 65k of NOR flash storage for application config data */
#define APP_CONFIG_REGION_SIZE              0x10000
#define NOR_APP_CONFIG_START_ADDR           (EBI_NOR_START_ADDR + (2 * UPDATE_REGION_SIZE))
#define FRAM_SIZE                          (0x40000)
#define FRAM_APP_SIZE                      (0x30100)
#define APP_START_ADDRESS                  (0x10000)
#define UPDATE_START_ADDRESS               (APP_START_ADDRESS - HEADER_REGION_SIZE)


#ifdef __cplusplus
}
#endif
    
#endif /* STORAGE_MEMORY_LAYOUT_H */
