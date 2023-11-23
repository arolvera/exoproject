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
 * 
 * @Company Exoterra
 * @FileName component_keys.h
 * @Summary Component (accessor) file names
 * @Description  This file contains the names of the flash and/or file system
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
 */

#ifndef _COMPONENT_KEYS_H   /* Guard against multiple inclusion */
#define _COMPONENT_KEYS_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif
    
#define FILENAME_LENGTH 2 /* One char + NULL delimiter */

/**
 * Translate the key into the region filename
 * @param key  the key (currently one letter)
 * @param fname pointer to store fname
 * @param region the region of the filename needed
 */
static inline void component_key_translate(const char *key, char *fname, int region)
{
    fname[0] = key[0] + region;
    fname[1] = 0;
}
   
/* These must remain in this order - ie Anode_2 = Anode_1++, and Anode_3 = Anode_2++*/
#define UPDATE_IMAGE              "A"
#define UNUSED_0                  "B"
    
#define ANODE_EXEC_HDR_1          "C"
#define ANODE_EXEC_HDR_2          "D"
#define ANODE_EXEC_HDR_3          "E"
#define ANODE_1                   "F"
#define ANODE_2                   "G"
#define ANODE_3                   "H"

#define KEEPER_EXEC_HDR_1         "I"
#define KEEPER_EXEC_HDR_2         "J"
#define KEEPER_EXEC_HDR_3         "K"    
#define KEEPER_1                  "L"
#define KEEPER_2                  "M"
#define KEEPER_3                  "N"
    
#define OUTER_MAGNET_EXEC_HDR_1   "O"
#define OUTER_MAGNET_EXEC_HDR_2   "P"
#define OUTER_MAGNET_EXEC_HDR_3   "Q"
#define OUTER_MAGNET_1            "R"
#define OUTER_MAGNET_2            "S"
#define OUTER_MAGNET_3            "T"

#define INNER_MAGNET_EXEC_HDR_1   "O"
#define INNER_MAGNET_EXEC_HDR_2   "P"
#define INNER_MAGNET_EXEC_HDR_3   "Q"
#define INNER_MAGNET_1            "R"
#define INNER_MAGNET_2            "S"
#define INNER_MAGNET_3            "T"

#if STORAGE_LAYOUT == STORAGE_LAYOUT_V1    
#define APP_RECOVERY_EXEC_HDR_1   "U"
#define APP_RECOVERY_EXEC_HDR_2   "V"
#define APP_RECOVERY_EXEC_HDR_3   "W"
#define APP_RECOVERY_1            "X"
#define APP_RECOVERY_2            "Y"
#define APP_RECOVERY_3            "Z" 
#endif

#define USER_CONFIG_VAR_EXEC_HDR_1 "U"
#define USER_CONFIG_VAR_EXEC_HDR_2 "V"
#define USER_CONFIG_VAR_EXEC_HDR_3 "W"
#define USER_CONFIG_VAR_1         "X"
#define USER_CONFIG_VAR_2         "Y"
#define USER_CONFIG_VAR_3         "Z" 

#define VALVES_EXEC_HDR_1         "a"
#define VALVES_EXEC_HDR_2         "b"
#define VALVES_EXEC_HDR_3         "c"
#define VALVES_1                  "d"
#define VALVES_2                  "e"
#define VALVES_3                  "f"

#define SYSTEM_CONTROL_EXEC_HDR_1 "g"
#define SYSTEM_CONTROL_EXEC_HDR_2 "h"
#define SYSTEM_CONTROL_EXEC_HDR_3 "i"
#define SYSTEM_CONTROL_1          "j"
#define SYSTEM_CONTROL_2          "k"
#define SYSTEM_CONTROL_3          "l"

#define BOOTLOADER_EXEC_HDR_1     "m"
#define BOOTLOADER_EXEC_HDR_2     "n"
#define BOOTLOADER_EXEC_HDR_3     "o"    
#define BOOTLOADER_1              "p"
#define BOOTLOADER_2              "q"
#define BOOTLOADER_3              "r"

/* For writing the application to the execution area */
#define APPLICATION_EXE           "s"

#define CONDITION_STATUS          "t"
#define TC_BOOTLOADER             "u"
#define UPDATER_TRIGGER           "v"
#define LOCKOUT_TIMER             "w"


#define INIT_MEM_COMP = {                                                                              \
{SYSTEM_CONTROL_1,  SYSTEM_CONTROL_EXEC_HDR_1,  MAGIC_EXECUTION_SYSTEM_CONTROL},/* System control */   \
    {KEEPER_1,          KEEPER_EXEC_HDR_1,          MAGIC_EXECUTION_KEEPER},        /* Keeper */       \
    {ANODE_1,           ANODE_EXEC_HDR_1,           MAGIC_EXECUTION_ANODE},         /* Anode */        \
    {OUTER_MAGNET_1,    OUTER_MAGNET_EXEC_HDR_1,    MAGIC_EXECUTION_MAGNET_O},      /* Magnet Outer */ \
    {INNER_MAGNET_1,    INNER_MAGNET_EXEC_HDR_1,    MAGIC_EXECUTION_MAGNET_I},      /* Magnet Inner */ \
    {VALVES_1,          VALVES_EXEC_HDR_1,          MAGIC_EXECUTION_VALVES},        /* Valves */       \
    {BOOTLOADER_1,      BOOTLOADER_EXEC_HDR_1,      MAGIC_EXECUTION_BOOTLOADER},    /* Bootloader */   \
};

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
