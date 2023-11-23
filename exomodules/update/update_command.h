/**
 * @file    update_command.h
 *
 * @brief   This is the interface for the command and control to update the system.
 * It has prepare (open the file), program (write a buffer), verify, and install
 * the image.
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

#ifndef UPDATE_COMMAND_H
#define	UPDATE_COMMAND_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "error/error_handler.h"


 
typedef struct update_error_specific_detail{
    unsigned int update_status;
} update_error_specific_detail_t; 


/**
 * @brief Intialize the Update Command module (PIO enable pin & interrupt)
 */
void uc_init(void);

/**
 * @brief Prepare for the update. Opens the files and saves the size of the
 * incoming file
 * @param size size of the update file
 * @return 0 on success, non-zero on failure. Will fail if file is too big or
 * the file is already open (two transfers started).
 */
int uc_prepare(int size);

/**
 * @brief Write a buffer to the update image area. The data in the buffer is a
 * smaller chunk of a larger message so the module tracks a position in the
 * file descriptor for the update image area. This will also close the file
 * once the transfer is complete.
 * @param buf buffer to write
 * @param size size of buffer
 * @return 0 on success, non-zero otherwise
 */
int uc_upload(void* buf, uint32_t size);

/**
 * @brief Verifies the update file stored in memory has been uploaded without
 * errors. Future upgrade may include decryption and authorization steps.
 * Assumes an update image file is sitting in SRAM update region.
 * @return 0 on success, non-zero otherwise
 */
int uc_verify(void);

/**
 * @brief Install an update image stored in SRAM. This involves writing the
 * full update image to NOR flash, writing the ECPK image to FRAM, and sending
 * the MVCP and ACP images to their respective clients. Assumes a valid image
 * is sitting in SRAM
 * @return 0 on success, non-zero otherwise
 */
int uc_install(void);

/**
 * @brief Start the newly installed update by resetting everything.
 * @return
 */
void uc_make_update_active(void);


//int uc_install_app(uint8_t abs_redundant_region);



#ifdef	__cplusplus
}
#endif

#endif	/* UPDATE_COMMAND_H */

