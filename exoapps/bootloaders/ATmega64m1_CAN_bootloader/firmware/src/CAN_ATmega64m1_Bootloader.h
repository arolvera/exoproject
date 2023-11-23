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
#ifndef ATMEGA64M1_CAN_BOOTLOADER_H
#define ATMEGA64M1_CAN_BOOTLOADER_H

/* Bootloader Version 1 - the first release into the wild */
#define BOOTLOADER_VERSION 4

#define  BL_CRIS_ID_ANODE    0x7F8
#define  BL_NODE_ID_ANODE     0x80

#define  BL_CRIS_ID_KEEPER   0x7F0
#define  BL_NODE_ID_KEEPER    0x40

#define  BL_CRIS_ID_VALVES   0x7E8
#define  BL_NODE_ID_VALVES    0x20

#define  BL_CRIS_ID_MAGNET_O 0x7E0
#define  BL_NODE_ID_MAGNET_O  0x10

#define  BL_CRIS_ID_MAGNET_I 0x7D8
#define  BL_NODE_ID_MAGNET_I  0x08

#define  BL_CRIS_ID_BUCK     0x7C0
#define  BL_NODE_ID_BUCK      0x08

/* Reply indexes for all the different contexts    */
#define BL_RPLY_COMMAND_CODE_INDEX    0x00
#define BL_RPLY_INDEX_READBACK_STATUS 0x00
#define BL_RPLY_INDEX_PROG_DATA       0x00
/* Bootloader version field in initial response    */
#define BL_RPLY_INDEX_BL_VERSION      0x00
/* App version field for application header        */
#define BL_RPLY_INDEX_APP_VERSION     0x03
/* Status for the ID SELECT NODE response          */
#define BL_RPLY_INDEX_INIT_STATUS     0x01
/* image CRC field */
#define BL_RPLY_INDEX_READBACK_CRC    0x06
/* image size field */
#define BL_RPLY_INDEX_IMAGE_SIZE      0x04
/* image version field */
#define BL_RPLY_INDEX_APP_NEW_VERSION 0x02
/* Bootloader header status */
#define BL_RPLY_INDEX_BL_HDR_STATUS   0x07

typedef enum {
    BL_HDR_OK,
    BL_HDR_CRC_MISMATCH        = 0x01,
    BL_HDR_IMAGE_SIZE_MISMATCH = 0x02,
    BL_HDR_IMAGE_CRC_MISMATCH  = 0x04,
    BL_HDR_IMAGE_UNINITIALIZED = 0x08,
    BL_HDR_INVALID_MAGIC       = 0x10,
    BL_HDR_READ_ERR            = 0x20,
    BL_HDR_UNKNOWN_ERR         = 0xFF
} bootloader_header_err_t;

/* All command except intial ID node select status */
#define BL_RPLY_STATUS_INDEX       0x00

#define BL_RPLY_READBACK_CRC_INDEX 0x01

// Command codes received from host (FROM bootloader server)
#define BL_CMD_PROG_INIT                        0x00
#define BL_CMD_ERASE_FLASH                      0x80
#define BL_CMD_READ_DATA                        0x00
#define BL_CMD_BLANK_CHECK                      0x80
#define BL_CMD_START_APPLICATION                0x03

// Reply codes from client (TO the bootloader server)
#define BL_RPLY_PROG_DATA_OK_DONE                 0x00
#define BL_RPLY_CLIENT_COM_CLOSED                 0x00
#define BL_RPLY_CLIENT_COM_OPENED                 0x01
#define BL_RPLY_PROG_DATA_OK_MORE                 0x02
#define BL_RPLY_CMD_OK_XFER_CRC_FAILED            0x03
#define BL_RPLY_READBACK_DONE                     0x03
#define BL_RPLY_CMD_OK_READBACK_CRC_FAILED        0x04
#define BL_RPLY_CMD_OK_INVALID_ADDR               0x04
#define BL_RPLY_INVALID_STATE                     0x05

typedef enum
{
    IDLE                        = 0x00,
    NODE_COMMS_OPEN             = 0x01,
    PROGRAMMING_MODE_INITIALIZE = 0x02,
    PROGRAMMING                 = 0x03,
    PROGRAMMING_DONE            = 0x04,
    READING_MEMORY              = 0x05,
    BLANK_CHECKING              = 0x06,
    BLANK_CHECKED               = 0x07,
    ERASING                     = 0x08,
    ERASED                      = 0x09,
    PROGRAMMING_MODE_START      = 0x0A, // Start/End address set, no new data yet
    APPLICATION_START           = 0x0B
} bootloaderState_t;

/* The lower three bits are reserved for commands  */
#define CRIS_COMMAND_MASK   ( (~0x7) & (0X7ff) )
/* CRIS Commands - Embedded in lower three bits of CAN ID */
#define CRIS_COMMAND_ID_SELECT_NODE         0
#define CRIS_COMMAND_ID_PROG_START          1
#define CRIS_COMMAND_ID_PROG_DATA           2
#define CRIS_COMMAND_ID_DISPLAY_DATA        3
#define CRIS_COMMAND_ID_START_APP           4
#define CRIS_COMMAND_ID_SELECT_MEM_PAGE     6
#define CRIS_COMMAND_ID_NEW_BOOT            7

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma pack(push, 1)
typedef struct bootloader_initial_msg {
    uint8_t  bootloader_version;
    uint32_t app_version;
    uint16_t app_crc;
    uint8_t  header_stat;
} bootloader_initial_msg_t;

typedef union bootloader_initial_info {
    bootloader_initial_msg_t msg;
    uint8_t data[8];
} bootloader_initial_info_t;
#pragma pack(pop)
#pragma GCC diagnostic pop

#endif