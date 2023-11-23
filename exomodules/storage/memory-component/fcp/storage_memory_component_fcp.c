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
/**
* storage_memory_interface.h
*
* @Company
* ExoTerra
*
* @File Name
* storage_memory_interface.h
*
* @Summary
* Defines APIs based on platform being compiled for, currently supported and
* tested are the Linux host and the SAMV71 target.  Also defined here is the
* component key map.  This is what ties together the atmega data files
* and their meta data.
*
*/

#ifndef _STORAGE_MEMORY_INTERFACE_H    /* Guard against multiple inclusion */
#define _STORAGE_MEMORY_INTERFACE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "mem_comp_map.h"
#include "common.h"
#include "component_keys.h"
#include "magic_numbers.h"

#ifdef __cplusplus
        extern "C" {
#endif

#if defined(__x86_64__) // Defined in Linux build used to test flash implementation with
            // filesystem instead of flash
#define storage_memory_read  read
#define storage_memory_write write
#define storage_memory_lseek lseek
#define storage_memory_fstat fstat
#define storage_memory_open  open
#define storage_memory_close close
#define storage_memory_stat stat
#endif


#if defined(__ATSAMV71Q21B__)
#include "storage_memory.h"
#define storage_memory_read  sm_read
#define storage_memory_write sm_write
#define storage_memory_lseek sm_lseek
#define storage_memory_fstat sm_fstat
#define storage_memory_open  sm_open
#define storage_memory_close sm_close
#define storage_memory_stat  sm_stat
#endif

#ifdef SD
// FIXME TDB
#endif

typedef struct component_key_map {
/* the bin file key */
const char *image_base_key;
/* the execution header */
const char *exhdr_base_key;
/* Exe header magic number */
uint32_t magic;
} component_key_map_t;



/* This structure may used when you do not care where the memory is, or if the
* memory is shared.  This is what you want if you are using the master
* component list and just want open a file.
*
* DO NOT use this for any memory scrubbing or any place where you are iterating
* over REAL Memory regions
*
* These elements must be in component_t enum order and have COMPONENT_MAXIMUM elements
*/

component_key_map_t component_map[]
= {
 [MEMCOMPONENT_SYSTEM_CONTROL] = {FLIGHT_CONTROL_1,  FLIGHT_CONTROL_EXEC_HDR_1,  MAGIC_EXECUTION_FLIGHT_CONTROL},/* System control */
 [MEMCOMPONENT_BOOTLOADER]     = {BOOTLOADER_1,      BOOTLOADER_EXEC_HDR_1,      MAGIC_EXECUTION_BOOTLOADER},    /* Bootloader */
 [MEMCOMPONENT_BUCK]           = {BUCK_1,            BUCK_EXEC_HDR_1,            MAGIC_EXECUTION_BUCK},        /* Keeper */
};

const int COMPONENT_MAP_ARRAY_SIZE = (sizeof(component_map)/sizeof(component_map[0]));


#define REDUNDANT_REGION_MASK ((1 << MEMCOMPONENT_MAX) - 1)

/*
* Use this structure for all memory scrubbing and recover routines
*
* It CAN and SHOULD be used whenever iterating over the mem_component enum
* and the ORDER of REAL memory areas matters.
*
*  These elements must be in mem_component_t enum order and have MEMCOMPONENT_MAX elements
*/
component_key_map_t mem_component_map[] = {
        [MEMCOMPONENT_SYSTEM_CONTROL] = {FLIGHT_CONTROL_1, FLIGHT_CONTROL_EXEC_HDR_1, MAGIC_EXECUTION_FLIGHT_CONTROL}, /* System control */
        [MEMCOMPONENT_BOOTLOADER]     = {BOOTLOADER_1, BOOTLOADER_EXEC_HDR_1, MAGIC_EXECUTION_BOOTLOADER},             /* Bootloader */
        [MEMCOMPONENT_BUCK]           = {BUCK_1, BUCK_EXEC_HDR_1, MAGIC_EXECUTION_BUCK},                               /* Keeper */
};

const int MEM_COMPONENT_MAP_ARRAY_SIZE = (sizeof(mem_component_map)/sizeof(mem_component_map[0]));


/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
End of File
*/
