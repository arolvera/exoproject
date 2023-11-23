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
 * Storage_memory.h
 * 
 * @Company
 * ExoTerra
 * 
 * @File Name
 * storage_memory.h
 * 
 * @Summary
 * Declares APIs for interacting with the embedded flash controller.
 * 
 * @Description
 * This module is designed to work like standard Linux file system calls
 * (open, read, write, etc.).  This enables us to keep the higher level routines
 * compatible with a Linux host machine, which allows us to do a great deal of
 * testing on the host and not suffer the handicaps associated with running on
 * the target.  It will also ease the pain of moving some 'things' to a
 * different storage device later.
 *   
 * Of course there is no real underlying file system.  Access to the 'things'
 * that are stored on the flash are done through 'KEYS', which are defined
 * in the component_keys.h file.   Each 'thing' is a component or an accessor.
 * Each component has a key (currently just an ascii character) that is used
 * to look up and allow access to that component's flash area.  A component/
 * accessor is anything that needs to store something in the embedded flash.
 * This can be one of the Atmega binary files and associated meta data, or
 * this application itself, or configuration settings.  This module is, and
 * must remain, agnostic to 'what' the 'thing' actually is.  This module
 * is only responsible for managing access to the EFC.
 * 
 * Storage memory layout is defined in storage_memory_layout.h.
 * 
 * This module is the only place the EFC should be touched directly.
 * 
 *
 */

#ifndef _STORAGE_MEMORY_H    /* Guard against multiple inclusion */
#define _STORAGE_MEMORY_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

int sm_erase16_pages(uint32_t addr);
ssize_t sm_read(int fd, void* buf, size_t count);
ssize_t sm_write(int fd, const void* buf, size_t count);
off_t sm_lseek(int fd, off_t offset, int whence);
int sm_fstat(int fd, struct stat* buf);
int sm_open(const char* path, int opts, ...);
int sm_close(int fd);
void sm_init(void );
int sm_flash_lock(int lock) __attribute((weak));
ssize_t sm_fflush(int fd);
int sm_stat(const char *filename, struct stat* buf);
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
