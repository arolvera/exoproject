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

#define BUCK_EXEC_HDR_1           "C"
#define BUCK_EXEC_HDR_2           "D"
#define BUCK_EXEC_HDR_3           "E"
#define BUCK_1                    "F"
#define BUCK_2                    "G"
#define BUCK_3                    "H"

#define FLIGHT_CONTROL_EXEC_HDR_1 "I"
#define FLIGHT_CONTROL_EXEC_HDR_2 "J"
#define FLIGHT_CONTROL_EXEC_HDR_3 "K"
#define FLIGHT_CONTROL_1          "L"
#define FLIGHT_CONTROL_2          "M"
#define FLIGHT_CONTROL_3          "N"

#define BOOTLOADER_EXEC_HDR_1     "O"
#define BOOTLOADER_EXEC_HDR_2     "P"
#define BOOTLOADER_EXEC_HDR_3     "Q"
#define BOOTLOADER_1              "R"
#define BOOTLOADER_2              "S"
#define BOOTLOADER_3              "T"

/* For writing the application to the execution area */
#define APPLICATION_EXE           "U"

#define TC_BOOTLOADER             "V"
#define UPDATER_TRIGGER           "W"



/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif

/* *****************************************************************************
End of File
*/

