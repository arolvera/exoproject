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
* Bootloader server
*
* @Company  Exoterra
* @File Name bootloader_server.c
* @Summary Bootloader server for client MCUs,
* @Description Server boot image to clients. The implementation is based on the
*          Slim CAN bootloader app note:
*          AVR076: AVR� CAN - 4K Boot Loader
*          https://www.microchip.com/wwwAppNotes/AppNotes.aspx?appnote=en591223
*/

#include "client_update_server.h"// This module's header

#include "definitions.h"// RTOS Queue Defines
#include "storage/storage_helper.h"
#include "trace/trace.h"
#include <string.h>   // memcpy
#include <sys/types.h>// System Types
#include "msg-handler/msg_handler.h"
#include "macro_tools.h"
#include "fram/fram_va41630_cmn.h"
#include "storage_memory_layout.h"
#include "mcu_include.h"

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



#define DEVICE_ID 0
#define APP_NODE_ID           0x22u

#define CLIENT_BOOT_REQUEST  0x780

#define CMD_SERVER_SEG_TFER_INIT          0x21
#define CMD_SERVER_SEG_TFER_LAST_SEG_BIT  0x01


#define CMD_CLIENT_SEG_TFER_INIT_ACK    0x60
#define CMD_CLIENT_SEG_TFER_DATA_ACK    0x20
#define CMD_SEG_TFER_TOGGLE_BIT         0x10

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


/* Bits to set in boot_status field of error detail */



#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Defines: Private definitions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/* Initial delay waiting for clients to announce their presence               */
#define BOOTLOADER_INITIAL_DELAY     ( 1000 / portTICK_PERIOD_MS )


#define COMPONENT_ID_TO_CLIENT_INDEX(__X__) (__X__ - 1)
/* ************************************************************************** */
/* ************************************************************************** */
/* RTOS: RTOS definitions                                                     */
/* ************************************************************************** */
/* ************************************************************************** */

static QueueHandle_t xBLQueue;
static StaticQueue_t xBLStaticQueue;

/* Total outstanding BL items       */
#define BUFFER_COUNT                    20
/* Data portion of SDO + Callback   */
#define QUEUE_ITEM_SIZE      sizeof(message_t)

/* Static task defines */
#define BL_TASK_STACK_SIZE   ( configMINIMAL_STACK_SIZE * 4 )
#define BL_TASK_PRIORITY     ( tskIDLE_PRIORITY )

#define BL_QUEUE_STATIC_SIZE (BUFFER_COUNT * QUEUE_ITEM_SIZE)
uint8_t ucBLQueueStorageArea[ BL_QUEUE_STATIC_SIZE * 2 ];

static int cus_callback(message_t* msg);


/* Call back pointer & message IDs to register for client messages */
msg_callback_t bootloader_client_node = {
        .node = {
                .range_low  = 0x580,
                .range_high = 0x58F,
                .left = NULL,
                .right = NULL,
        },
        .cb = cus_callback,
};

/* Call back pointer & message IDs to register for client messages */
msg_callback_t bootloader_client_error_node = {
        .node = {
                .range_low  = 0x81,
                .range_high = 0x8F,
                .left = NULL,
                .right = NULL,
        },
        .cb = cus_callback,
};

/* Buffer for the task being to use as its stack. */
StackType_t blStack[ BL_TASK_STACK_SIZE ];

/* Structure that will hold the TCB of the task being created. */
StaticTask_t blTaskBuffer;

#define CRC_16_POS_UPPER 5

#define MSG_SIZE 8
#define BL_RPLY_MSG(__BL_RPLY_BUF__, __COMMAND__) \
memcpy(__BL_RPLY_BUF__, __COMMAND__, MSG_SIZE);


typedef enum  {
    STATE_IDLE = 0,
    STATE_WAITING_INITIAL_BOOT,
    STATE_WAITING_COM_OPEN,
    STATE_PROG_START_ERASE,
    STATE_PROG_START_BLANK_CHECK,
    STATE_PROG_INIT,        // Set start & end address
    STATE_UPLOADING,        // Programming the data
    STATE_UPLOAD_DONE,    // Last pack sent
    STATE_APP_START,        // Kick off the app and hope for the best
    STATE_APP_SUCCESS,      // Bootload success, client should boot
    STATE_BOOT_ATTEMPTS_EXCEEDED,
    STATE_UPLOAD_DONE_READBACK_CHECK,
} client_states_t;

int (*CLIENT_CB_t)(void* arg);

typedef struct bl_error{
    uint8_t error_code;
    uint8_t e_byte_1;
    uint8_t e_byte_2;
    uint8_t e_byte_3;
    uint16_t crc;
    uint16_t line_number;
}bl_error_t;


typedef struct BootLoaderClients {
/* Current Client State */
    int state;
/* Component ID */
    int component;
/* Boot attempts */
    int boot_attempts;
/* Client's bootloader version */
    uint8_t bootloader_version;
/* Local file descriptor */
    int fd;
/* Position in programming file */
    uint32_t pos;
/* Programming file size        */
    uint32_t size;
/* save last error */
    bl_error_t bl_error;
/* version */
    uint32_t version;
/* crc */
    uint32_t crc;
/* This is the bootloader's initial info, version, crc, status, etc., which
* comes across in the first message  */
    bootloader_initial_info_t bl_info;
    message_t client_msg;
    int toggle_bit_state;
    int active;
    int first_packet;
} BootLoaderClient_t;

#define BOOTLOADER_CLIENT_INIT  {                                                 \
[COMM_ID_ANODE] = {STATE_IDLE, BL_CRIS_ID_ACP,  0,0,-1,0,0, .active = 1, .first_packet = 1},         \
[COMM_ID_MAGNET_O] = {STATE_IDLE, BL_CRIS_ID_MVP,   0,0,-1,0,0, .active = 1, .first_packet = 1}  };  \

static BootLoaderClient_t client_ops[] = BOOTLOADER_CLIENT_INIT


/**
* Callback function for process bootloader messages.  Packages up the message
* and places it in the bootloader queue.
*
* @param id  Message ID from client
* @param data pointer to message data
* @param dlc data length code
* @return 0
*/
static int cus_callback(message_t* msg)
{
    int err = 0;
    xQueueSend(xBLQueue, msg, 0);
    return err;
}


static int verify_hdr(int fd, UpdateFileHeader_t* uh)
{
    int err = 0;

    /* Validate the header */
    fram_read(UPDATE_START_ADDRESS, (uint8_t*)uh, sizeof(UpdateFileHeader_t));
    if(uh->info.magic_number != MAGIC_NUMBER_UPDATE_HDR){
        err = __LINE__;
    } else {
        uint16_t crc = 0;
        for (uint16_t i = 0; i < uh->info.header_size - sizeof(uh->crc); i++) {
            crc = update_crc_16(crc, ((uint8_t *)uh)[i]);
        }
        if (crc != uh->crc) {
            err = __LINE__;
        }
    }

    return err;
}



static int client_image_transfer_process(int client_id, uint8_t* msg)
{
    int err = 0;

    if(client_id < 0 || client_id > (int)(sizeof(client_ops)/sizeof(client_ops[0]))){
        err = __LINE__;
    } else {
        if(client_ops[client_id].state == STATE_UPLOADING){
            uint8_t toggle_bit = msg[0] & CMD_SEG_TFER_TOGGLE_BIT;
            if(toggle_bit != client_ops[client_id].toggle_bit_state){
                err = __LINE__;
            } else {
                int n = 7;
                if(client_ops[client_id].pos + n >= client_ops[client_id].size){
                    n = client_ops[client_id].size - client_ops[client_id].pos;
                    client_ops[client_id].client_msg.data[0] |= CMD_SERVER_SEG_TFER_LAST_SEG_BIT;
                    client_ops[client_id].state = STATE_UPLOAD_DONE_READBACK_CHECK;
                }
                storage_memory_read(client_ops[client_id].fd, &client_ops[client_id].client_msg.data[1], n);
                client_ops[client_id].pos += n;
                if(!client_ops[client_id].first_packet) {
                    client_ops[client_id].toggle_bit_state ^= CMD_SEG_TFER_TOGGLE_BIT;
                }
                client_ops[client_id].first_packet = 0;
                client_ops[client_id].client_msg.data[0] &= ~CMD_SEG_TFER_TOGGLE_BIT;
                client_ops[client_id].client_msg.data[0] |= client_ops[client_id].toggle_bit_state;
            }
        } else if(client_ops[client_id].state == STATE_UPLOAD_DONE_READBACK_CHECK){
            /* We're done reading from this, so cleanup */
            storage_memory_close(client_ops[client_id].fd);
            /* Request Client verification */
            /* If verify succeeds, send it! */
        } else {
            err = __LINE__;
        }
    }


    return err;
}



static int cus_error_set(int client_id, uint8_t* error_info)
{
    int err = 0;
    if(error_info == NULL || client_id > (int)(sizeof(client_ops)/sizeof(client_ops[0])) || !client_ops[client_id].active){
        err = __LINE__;
    } else {
        memcpy(&client_ops[client_id].bl_error, error_info, sizeof(client_ops[client_id].bl_error));
    }
    return err;
}


static int cus_reset(unsigned int client_id)
{
    int err = 0;
    if(client_id > sizeof(client_ops)/sizeof(client_ops[0]) || !client_ops[client_id].active){
        err = __LINE__;
    } else {
        if (client_ops[client_id].fd != -1) {
            storage_memory_close(client_ops[client_id].fd);
        }

        BootLoaderClient_t temp[sizeof(client_ops) / sizeof(client_ops[0])] = BOOTLOADER_CLIENT_INIT;
        memcpy(&client_ops, &temp, sizeof(client_ops));
    }
    return err;
}


/**
* Bootloader server task
* Waits on a message queue and process commands.  Does the work of parsing
* the client id, CRIS id, and the client look up, then calls the function
* that services the client (bootloader_service_client).
*
* As of right now, this is FreeRTOS specific.  The RTOS should be abstracted
* here, so that all this is portable.
*
* @param pvParameters pointer to task parameters.
*/
static void cus_task(void *pvParameters)
{
    int err = 0;
    BaseType_t xStatus = pdPASS;
    message_t message;
    TickType_t delay = BOOTLOADER_INITIAL_DELAY;

    while (1) {
        xStatus = xQueueReceive(xBLQueue, &message, delay);
        if (xStatus == pdFALSE) {
            // TODO handle a timeout waiting for clients
        } else {
            uint32_t client_id = message.id & 0xF;

            if ((message.id & 0x7F0) == BL_EMCY) {
                cus_reset(client_id);
                cus_error_set(client_id, message.data);
            } else if(client_ops[client_id].state == STATE_UPLOADING) {
                err = client_image_transfer_process(client_id, message.data);
                if (err) {
                    /* format error msg */

                }
                send_msg(client_ops[client_id].client_msg.id,
                         client_ops[client_id].client_msg.data,
                         client_ops[client_id].client_msg.dlc,
                         0);
            }
        }
    }
}




static int cus_client_image_invalidate(int client_id)
{
    int err = 0;
    char invalidate_password[] = "bensgr8";
    if(client_id < 0 || client_id >= (int)SIZEOF_ARRAY(client_ops) || !client_ops[client_id].active){
        err = __LINE__;
    } else {
        client_ops[client_id].client_msg.id = COMMAND_PARAMETERS_ID_BASE | client_id;
        client_ops[client_id].client_msg.dlc = 8;
        memcpy(client_ops[client_id].client_msg.data, invalidate_password, sizeof(invalidate_password));
        send_msg(client_ops[client_id].client_msg.id,
                 client_ops[client_id].client_msg.data,
                 client_ops[client_id].client_msg.dlc,
                 0);
    }
    return err;
}


static int cus_client_reset(int client_id)
{
    int err = 0;
    if(client_id < 0 || client_id >= (int)SIZEOF_ARRAY(client_ops) || !client_ops[client_id].active){
        err = __LINE__;
    } else {
        client_ops[client_id].client_msg.id = COMMAND_PARAMETERS_ID_BASE | client_id;
        client_ops[client_id].client_msg.dlc = 8;
        client_ops[client_id].client_msg.data[0] = 0xFF;
        send_msg(client_ops[client_id].client_msg.id,
                 client_ops[client_id].client_msg.data,
                 client_ops[client_id].client_msg.dlc,
                 0);
    }
    return err;
}


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/**
* Intialize tasks and queues.  Thiw will spin up a task that waits on a command
* queue.   Responses are handled through a callback function that comes in the
* command queue.
* @return 0 on success, error otherwise (always successfully for now)
*/
int cus_init(void)
{
    /* Create the queues */
    xBLQueue = xQueueCreateStatic( BUFFER_COUNT, QUEUE_ITEM_SIZE,
                                   ucBLQueueStorageArea, &xBLStaticQueue );

    /* Create File Transfer Tasks */
    xTaskCreateStatic(cus_task, "Bootloader Server Task",
                      BL_TASK_STACK_SIZE, 0, BL_TASK_PRIORITY, blStack, &blTaskBuffer);

    msg_handler_register_callback(&bootloader_client_node);
    msg_handler_register_callback(&bootloader_client_error_node);

    return 0;
}


int cus_client_prepare(int client_id)
{
    int err = 0;
    if(client_id >= 0 && client_id < COMPONENT_MAXIMUM) {
        cus_client_image_invalidate(client_id);
        cus_client_reset(client_id);
    } else {
        err = __LINE__;
    }

    return err;
}


int cus_image_request(int client_id)
{
    int err = 0;

    /* Reset clients boot parameters */
    BootLoaderClient_t temp[sizeof(client_ops)/sizeof(client_ops[0])] = BOOTLOADER_CLIENT_INIT;
    memcpy(&client_ops[client_id], &temp[client_id], sizeof(BootLoaderClient_t));
    /* Flag to control bit toggling on first packet */
    client_ops[client_id].first_packet = 1;
    client_ops[client_id].state = STATE_WAITING_INITIAL_BOOT;
    if(client_id >= 0 && client_id < (int)(sizeof(client_ops)/sizeof(client_ops[0])) && client_ops[client_id].active) {
        UpdateFileHeader_t uh = {0};
        client_ops[client_id].fd = storage_memory_open((const char *) FRAM_UPDATE_IMAGE, O_RDONLY);
        if( client_ops[client_id].fd != -1){
            err = verify_hdr(client_ops[client_id].fd, &uh);
        }

        if(!err){
            client_ops[client_id].size = uh.image_hdr[0].image_info.size + sizeof(uh);
            client_ops[client_id].client_msg.id = SEG_TFER_SVR_ID | client_id;
            client_ops[client_id].client_msg.dlc = 8;
            client_ops[client_id].client_msg.data[0] = CMD_SERVER_SEG_TFER_INIT;
            client_ops[client_id].state = STATE_UPLOADING;
        }
        send_msg(client_ops[client_id].client_msg.id,
                 client_ops[client_id].client_msg.data,
                 client_ops[client_id].client_msg.dlc,
                 0);
        client_ops[client_id].client_msg.data[0] = 0;
    } else {
        err = __LINE__;
    }
    return err;
}


/* *****************************************************************************
End of File
*/
