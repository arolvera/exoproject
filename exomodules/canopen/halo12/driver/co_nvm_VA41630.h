/**
 * @file    co_nvm_VA41630.h
 *
 * @brief   ??? Defines an extern nvm driver object. This is a container for nvm init,
 * read, and write functions statically defined in the c file.
 *
 * Under what conditions does canopen need read/write access to NVM? Is it to write
 * software updates? Configuration data?
 * No known use case. Could get rid of? Or define a use case.
 *
 * The init, read, and write functions are not implemented. Is there another method for
 * receiving updates and writing to NVM? Should we be using these functions from the
 * storage module?????:
 *   storage_memory_read
 *   storage_memory_write
 *   storage_memory_lseek
 *   storage_memory_fstat
 *   storage_memory_open
 *   storage_memory_close
 *   storage_memory_stat
 *
 *   NOOOOOOO
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

#ifndef CO_NVM_VA41630_H
#define CO_NVM_VA41630_H

#ifdef __cplusplus               /* for compatibility with C++ environments  */
extern "C" {
#endif

#include "co_if.h"



/* TODO: rename the extern variable declaration to match the naming convention:
 *   <device-name>NvmDriver
 *   Do you mean rename ATSAMV71 to VA41630 ?????????????????????????????????????????????
 */
extern const CO_IF_NVM_DRV ATSAMV71NvmDriver;



#ifdef __cplusplus               /* for compatibility with C++ environments  */
}
#endif

#endif  // CO_NVM_VA41630_H
