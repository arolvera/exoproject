/* Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

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


#include "definitions.h" /* defines EFC_ APIs */
#include "utils/stack.h"
#include "utils/macro_tools.h"
#include "utilities/trace/trace.h"
#include "storage/storage_class.h"
#include "storage/storage_memory_interface.h"
//#include "storage/storage_memory_halo_sam.h"
#include "magic_numbers.h"
#include "storage/memory-component/halo6/storage/component_keys.h"
#include "storage_memory_layout.h"

#ifndef WATCHDOG_CLEAR
#define WATCHDOG_CLEAR()
#endif

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

#define ERASE_PAGE_MINIMUM (16)
#define ERASE_SIZE (ERASE_PAGE_MINIMUM * IFLASH_PAGE_SIZE)
uint8_t erase_buffer[ERASE_SIZE];
/*
 * storage_memory_interface global defines
 */
component_key_map_t component_map[] = {
    {SYSTEM_CONTROL_1,  SYSTEM_CONTROL_EXEC_HDR_1,  MAGIC_EXECUTION_SYSTEM_CONTROL},/* System control */
    {KEEPER_1,          KEEPER_EXEC_HDR_1,          MAGIC_EXECUTION_KEEPER},        /* Keeper */
    {ANODE_1,           ANODE_EXEC_HDR_1,           MAGIC_EXECUTION_ANODE},         /* Anode */
    {OUTER_MAGNET_1,    OUTER_MAGNET_EXEC_HDR_1,    MAGIC_EXECUTION_MAGNET_O},      /* Magnet Outer */
    {INNER_MAGNET_1,    INNER_MAGNET_EXEC_HDR_1,    MAGIC_EXECUTION_MAGNET_I},      /* Magnet Inner */
    {VALVES_1,          VALVES_EXEC_HDR_1,          MAGIC_EXECUTION_VALVES},        /* Valves */
    {BOOTLOADER_1,      BOOTLOADER_EXEC_HDR_1,      MAGIC_EXECUTION_BOOTLOADER},    /* Bootloader */
};
const int COMPONENT_MAP_ARRAY_SIZE = (sizeof(component_map)/sizeof(component_map[0]));

/*
 * Use this structure for all memory scrubbing and recover routines
 *
 * It CAN and SHOULD be used whenever iterating over the mem_component enum
 * and the ORDER of REAL memory areas matters.
 *
 *  These elements must be in mem_component_t enum order and have MEMCOMPONENT_MAX elements
 */
component_key_map_t mem_component_map[] = {
    {SYSTEM_CONTROL_1,  SYSTEM_CONTROL_EXEC_HDR_1,  MAGIC_EXECUTION_SYSTEM_CONTROL},/* System control */
    {ANODE_1,           ANODE_EXEC_HDR_1,           MAGIC_EXECUTION_ANODE},         /* Anode */
    {KEEPER_1,          KEEPER_EXEC_HDR_1,          MAGIC_EXECUTION_KEEPER},        /* Keeper */
    {OUTER_MAGNET_1,    OUTER_MAGNET_EXEC_HDR_1,    MAGIC_EXECUTION_MAGNET_O},      /* Magnet Outer */
    {VALVES_1,          VALVES_EXEC_HDR_1,          MAGIC_EXECUTION_VALVES},        /* Valves */
    {BOOTLOADER_1,      BOOTLOADER_EXEC_HDR_1,      MAGIC_EXECUTION_BOOTLOADER},    /* Bootloader */
#if UCV_ENABLE
    {USER_CONFIG_VAR_1, USER_CONFIG_VAR_EXEC_HDR_1, MAGIC_EXECUTION_UCV},           /* User Config Vars */
#endif
};
const int MEM_COMPONENT_MAP_ARRAY_SIZE = (sizeof(mem_component_map)/sizeof(mem_component_map[0]));


