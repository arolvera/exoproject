/**
 * @file    storage_memory_interface.h
 *
 * @brief   This file contains the names of the flash and/or file system
 * file names. The built-in flash has file system abstraction.  All access is
 * through these file names using standard linux file system read/write/seek
 * functions and file descrictors.  This enables us to do unit testing of the
 * update (and other) process on the host machine.
 *
 * The filenames below are treated as KEYS in some places.  There are a few
 * requirements for the filename/KEYS below.  IF the keys ever change, the new
 * KEYS must meet the following requirements.
 * 1. They must remain in the same sorted order. (currently A-Z,a-z)
 * 2. The KEY for like 'things' must be able to be incremented to access the
 *    next related 'thing'.
 *    For example:
 *      The Anode data file has three copied ANODE_1, ANODE_2, & ANODE_3.
 *      The key for the first file is 'F', the second is 'G', and then 'H'.
 *      That allows us to start with the first file, and then just increment
 *      the key to access the next related thing, the next copy of the ANODE
 *      data file in this case.
 * 3. The KEY must facilitate fast component info look up in the storage memory
 *    module.  Current the KEY is converted to an offset in the component array
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

#ifndef COMPONENT_KEYS_H
#define COMPONENT_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Keys used for componenet maps that map memory address and
 * functions for manipulating data. Backups point to NOR backup regions
 * 1 and 2. Non-backup point to the update region of SRAM.
 */
#define UPDATE_IMAGE                "A"
#define UPDATE_IMAGE_BKUP1          "B"
#define UPDATE_IMAGE_BKUP2          "C"
#define FRAM_UPDATE_IMAGE           "D"
#define APP_CONFIGURATION           "E"

//These are defined just to make master image constructor happy. THey are not used elsewhere
// This is not true. It is used in client lockout. What is the deal here?
// ToDo: figure out what to do with these
#define LOCKOUT_TIMER             ((char *)0xF)
#define CONDITION_STATUS          ((char *)0xF)
#define TC_BOOTLOADER             ((char *)0xF)
#define BOOTLOADER_EXEC_HDR_1     ((char *)0xF)
#define BOOTLOADER_1              ((char *)0xF)



#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_KEYS_H */

