
/**
 * @file    update_helper.h
 *
 * @brief   This module does the work of updating the system.
 *
 * @Description
 * Here are the routines to verify and validate the update image and to install
 * the included components into their designated locations. All device have
 * a 'KEY' that corresponds to filename or memory region on the host and a
 * designated storage location on the target machine.  This module is agnostic
 * to which system it is running on: host or target.
 *
 * At a high level, the functions in this module support the 4 update command
 * sub-indexes; Upload, Verify, Install, Enable. See design documentation for
 * more detail.
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

#ifndef UPDATE_HELPER_H
#define UPDATE_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "master_image_constructor.h"


#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */
typedef struct exe_header {
    uint32_t magic;                 /* 0x00-0x03 This Components Magic number  */
    UpdateImageHdrInfo_t component;          /* 0x04-0x27 Component info                */
    uint16_t num_copies;            /* 0x28-0x29 Which copy is active          */
    uint16_t active_copy;           /* 0x2A-0x2B Which copy is active          */
    uint16_t component_crc;         /* 0x2C-0x2D CRC for component image data  */
    uint8_t unused[0x10];           /* 0x38-0x3D Padding                       */
    uint16_t crc;                   /* 0x3E-0x3F CRC for this component        */
} exe_header_t;
#pragma pack(pop)                  /* restore original alignment from stack   */



/**
 * Calculate the 32 bit CRC for the given size
 * @param fd open fd (file pos at zero)
 * @param size the size to check
 * @param crc pointer to store crc
 * @return 0 on success or -1 on failure
 */
int uh_image_crc_calc16(int fd, int size, uint16_t *crc);

/**
 * Perform a CRC16 check on the provided buffer, size, and crc to match
 * @param buf buffer to perform the CRC check on
 * @param size size of the data in the buffer to check
 * @param match_crc crc to match
 * @return 0 if it matches, non-zero otherwise
 */
int uh_crc16_check(uint8_t *buf, int size, uint16_t match_crc);

/**
 * @brief Perform a CRC32 check on the provided buffer, size, and crc to match
 * @param buf buffer to perform the CRC check on
 * @param size size of the data in the buffer to check
 * @param match_crc crc to match
 * @return 0 if it matches, non-zero otherwise
 */
int uh_crc32_check(uint8_t *buf, int size, uint32_t match_crc);

/**
 * @brief The update binary has a header region and three binary regions; one for
 * each device (ECPK, MVCP, and ACP). This function computes the crc's for all four
 * regions and compares the values stored in the header.
 * @note This is one of two functions that splits up uh_update_software
 * @return 0 for match, non-zero if any one crc check fails
 */
int uh_update_crc_inspect(void);

/**
 * @brief Erase the Update region of SRAM.
 * @return 0 = success, non-zero otherwise
 */
int uh_update_region_erase(void);

/**
 * @brief Upload an update file from the stale region of the NOR software backup
 * into SRAM. Inspect the CRC's to ensure there was a valid image.
 * @return 0 if successfully loaded and there was a valid image, non-zero otherwise
 */
int uh_upload_stale_update(void);

/**
 * @brief Download an update file from SRAM to NOR Flash stale region. Inspect the
 * CRC's to ensure there was a valid image.
 * @return 0 = success, non-zero otherwise
 */
int uh_store_update_image(void);

/**
 * @brief Copy the ECPK image from SRAM update region to FRAM. Inspect the
 * CRC's to ensure there was a valid image.
 * @return 0 = success, non-zero otherwise
 */
int uh_install_ecpk_image(void);

/**
 * @brief Transmit the update to the client.
 * @return
 */
int uh_send_update_to_clients(uint8_t device_type);



#ifdef __cplusplus
}
#endif

#endif /* UPDATE_HELPER_H */
