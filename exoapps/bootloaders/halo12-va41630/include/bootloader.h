//
// Created by exolab on 10/18/23.
//

#ifndef SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_BOOTLOADER_H
#define SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_BOOTLOADER_H
#include "can/hal_can.h" /* for message_t */
#include "master_image_constructor.h" /* for UpdateFileHeader_t */
#include "mcu_include.h"
#include "error_codes.h"
#include "storage/storage_memory_layout.h"

#define SEG_TFER_DATA_LEN 7
#define VA41630_STACK_POINTER 0x20000000


#define CMD_CLIENT_BIT_TOGGLE(__CMD__) (*__CMD__ = (*__CMD__ ^ CMD_SEG_TFER_TOGGLE_BIT) | CMD_CLIENT_SEG_TFER_DATA_ACK);


#define SEG_TFER_FIRST_CLIENT_ACK                          \
{.id = SEG_TFER_CLT_ID,                                    \
 .data = {0x60, 0x00, 0x55, 0x01, 0x00, 0x00, 0x00, 0x00}, \
 .dlc = 8};

#define BOOTLOADER_NEW_IMAGE_REQUEST                       \
{.id = 0x280,                                              \
 .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, \
 .dlc = 8};

#define BOOTLOADER_EMCY                                    \
{.id = 0x80,                                              \
 .data = {ERROR_CODE_MCU_BOOT_ERR, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00}, \
 .dlc = 8};

/* Link definitions from  */
int tx_msg(int handle, message_t* msg);
int rx_msg(int handle, message_t* msg);
int comm_init(void);
int backup_image_install(void);
#endif //SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_BOOTLOADER_H
