/**
 * @file    bootloader_server.h
 *
 * @brief   ??? Bootloader server for client MCUs. Is this relevant to Halo 12?
 *
 * Delete this. Jacob will check in something relevant to Halo12.
 *
 * @Description Server boot image to clients. The implementation is based on the
 *          Slim CAN bootloader app note:
 *          AVR076: AVR� CAN - 4K Boot Loader
 *          https://www.microchip.com/wwwAppNotes/AppNotes.aspx?appnote=en591223
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

#ifndef BOOTLOADER_SERVER_H
#define BOOTLOADER_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"

int bootloader_server_init(void) __attribute__((weak));
void bootloader_client_init(void) __attribute__((weak));
int bootloader_server_callback(message_t* msg);
/* Provide C++ Compatibility */
    
#ifdef __cplusplus
}
#endif

#endif /* BOOTLOADER_SERVER_H */
