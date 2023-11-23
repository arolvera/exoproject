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

#include "bootloader_server.h"// This module's header
#include "CAN_ATmega64m1_Bootloader.h"
#include "client_control.h"
#include "definitions.h"// RTOS Queue Defines
#include "storage/storage_helper.h"
#include "trace/trace.h"
#include <string.h>   // memcpy
#include <sys/types.h>// System Types
#include "mcan/mcan_task.h"

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

/* Bootloader's internal message structure for queue boot client messages */
typedef struct bootload_msg {
int cris_id;
uint8_t data[8];
CAN_MSG_CALLBACK bootloader_callback;
} bootload_msg_t;

/* Total outstanding BL items       */
#define BUFFER_COUNT                    20
/* Data portion of SDO + Callback   */
#define QUEUE_ITEM_SIZE      sizeof(bootload_msg_t)

/* Static task defines */
#define BL_TASK_STACK_SIZE   ( configMINIMAL_STACK_SIZE * 4 )
#define BL_TASK_PRIORITY     ( tskIDLE_PRIORITY )

#define BL_QUEUE_STATIC_SIZE (BUFFER_COUNT * QUEUE_ITEM_SIZE)
uint8_t ucBLQueueStorageArea[ BL_QUEUE_STATIC_SIZE * 2 ];

int bootloader_server_callback(message_t* msg);

/* This is CAN address space reserved for boot loader clients.  Each client has
* a base ID, then the command/state information is encoded in the lower three
* bits of the 11-bit CAN ID */
#define BOOTLOADER_CLIENT_START_ID  0x7D8
#define BOOTLOADER_CLIENT_MAX_ID    0x7FF

/* Call back pointer & message IDs to register for client messages */
msg_callback_t bootloader_node = {
.node = {
.range_low  = BOOTLOADER_CLIENT_START_ID,
.range_high = BOOTLOADER_CLIENT_MAX_ID,
.left = NULL,
.right = NULL,
},
.cb = bootloader_server_callback,
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
STATE_WAITING_INITIAL_BOOT = 0,
STATE_WAITING_COM_OPEN,
STATE_PROG_START_ERASE,
STATE_PROG_START_BLANK_CHECK,
STATE_PROG_INIT,        // Set start & end address
STATE_PROG_DATA,        // Programming the data
STATE_PROG_DATA_END,    // Last pack sent
STATE_APP_START,        // Kick off the app and hope for the best
STATE_APP_SUCCESS,      // Bootload success, client should boot
STATE_BOOT_ATTEMPTS_EXCEEDED,
STATE_PROG_DONE_READBACK_CHECK,
} client_states_t;

typedef struct BootLoaderClients {
/* Clients Unique ID */
int client_id;
/* Current Client State */
int state;
/* Component ID */
mem_component_t component;
/* Boot attempts */
int boot_attempts;
/* Client's bootloader version */
uint8_t bootloader_version;
/* Local file descriptor */
int fd;
/* Position in programming file */
uint16_t pos;
/* Programming file size        */
uint16_t size;
/* save last error */
int last_error;
/* version */
uint32_t version;
/* crc */
uint32_t crc;
/* This is the bootloader's initial info, version, crc, status, etc., which
* comes across in the first message  */
bootloader_initial_info_t bl_info;
} BootLoaderClient_t;

#define BOOTLOADER_CLIENT_INIT  {                                                        \
/* Client array - must match client pos enums */                                     \
{BL_CRIS_ID_KEEPER,   STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_KEEPER,  0,0,-1,0,0}, \
{BL_CRIS_ID_ANODE,    STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_ANODE,   0,0,-1,0,0}, \
{BL_CRIS_ID_MAGNET_O, STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_MAGNETS, 0,0,-1,0,0}, \
{BL_CRIS_ID_MAGNET_I, STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_MAGNETS, 0,0,-1,0,0}, \
{BL_CRIS_ID_VALVES,   STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_VALVES,  0,0,-1,0,0}, \
{0,0},                                                                               \
{BL_CRIS_ID_BUCK,     STATE_WAITING_INITIAL_BOOT, MEMCOMPONENT_BUCK,    0,0,-1,0,0}, \
{0, 0} /* END OF LIST*/                                                              \
}

BootLoaderClient_t clients[] = BOOTLOADER_CLIENT_INIT;
/**
* Re-initialize the client structure.  This is called when the clients are
* powered off.  Therefore, if the boot attempts are exceed, the client must
* be shutdown and restarted
*/
inline void bootloader_client_init(void)
{
/* Make sure to close all open file descriptors when re-initing client struct */
int size = SIZEOF_ARRAY(clients);
for(int i = 0; i < size; i++){
if(clients[i].client_id != 0 && clients[i].fd >= 0){
 sm_close(clients[i].fd);
}
}
BootLoaderClient_t tmp_clients[] = BOOTLOADER_CLIENT_INIT;
memcpy(clients, tmp_clients, sizeof(clients));
}

#define READBACK_SUPPORT 3

/* Open/Close comms - 0 is node id */
#define CMD_OPEN_COMS_NODE_POS 0

static uint8_t command_open_comms[] =
{0,0,0,0,0,0,0,0};

/* Erase Flash - Atmega knows where to start, 1 & 2 specify erase 0xE000 */
static uint8_t command_erase[] =
{BL_CMD_ERASE_FLASH, 0xE0,0,0,0,0,0,0};

/* Check flash is erased to size in 3 (upper 8) & 4 (lower 8), which is based binary size */
static uint8_t command_blank_check[] =
{BL_CMD_BLANK_CHECK, 0x00, 0x00, 0xFF /* Size upper 8 */, 0xFF /* size lower 8*/, 0,0,0};

#define CMD_FLASH_END_POS_UPPER 3
#define CMD_FLASH_END_POS_LOWER 4
/*  0 hardcode to 0, 1 & 2 start at address 0, 3 & 4 is end address */
static uint8_t command_prog_init[] =
{BL_CMD_PROG_INIT, 0x00, 0x00, 0xFF, 0xFF, 0x01/* FIXME - Why is this here? */,0,0};

/* READ_DONE - 0 is command, 3 (upper) & 4(lower) are end address */
static uint8_t command_app_start[] =
{BL_CMD_START_APPLICATION, 0,0,0,0,0,0,0};

/* Response to a read  0 is command, 3 (upper) & 4(lower) are end address*/
static uint8_t command_read_check[] =
{BL_CMD_READ_DATA, 0x00, 0x00, 0xFF, 0xFF,0,0,0};

typedef struct client_message {
/* Message from client */
bootload_msg_t *pMessage;
/* client id (CAN) */
int client_id;
/* node it (in message) */
int node_id;
/* client command */
int command;
/* reply message */
uint8_t data[8];
} client_message_t;


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */


static void bootloader_server_client_boot_err(BootLoaderClient_t *pClient, uint32_t boot_err)
{
client_boot_err(boot_err);
bootloader_client_init();
}

/**
* Service the boot loader client.
*
* This takes a pointer to a client and services it according the the current
* command and clients state.  Client response info will be stored in the
* client message structure pointer.
*
*
* @param pClient pointer to the clients state information
* @param pCmsg pointer to client response information
*/

#define CONFIG_MCU_ALWAYS_BOOTLOAD 0
static void bootloader_service_client(BootLoaderClient_t *pClient, client_message_t *pCmsg)
{
/* The base of the response id with lower 3 bits cleared */
int cris_id = pCmsg->client_id;
/* Message from client */
uint8_t *client_msg = pCmsg->pMessage->data;
/* This service call's response to the client */
uint8_t *reply_msg = pCmsg->data;

int err = 0;
stat_helper_t buf = {0};
switch(pCmsg->command) {
/* Open COMS, start programming, and/or close COMS */
case CRIS_COMMAND_ID_SELECT_NODE :
case CRIS_COMMAND_ID_NEW_BOOT :
{
 /* Time to open COMS */
 if(pClient->state == STATE_BOOT_ATTEMPTS_EXCEEDED) {
     err = __LINE__;
     TraceE3(TrcMsgBLServ, "Boot attempt exceeded:%d",
             pClient->boot_attempts, 0,0,0,0,0);
 } else if(pClient->state == STATE_WAITING_INITIAL_BOOT) {
     /* Save the client initial info */
     memcpy(pClient->bl_info.data, client_msg,
            sizeof(pClient->bl_info.data));

     pClient->boot_attempts += 1;
     if(pClient->boot_attempts > 3) {
         err = __LINE__;
         pClient->state = STATE_BOOT_ATTEMPTS_EXCEEDED;
         TraceE3(TrcMsgBLServ, "Too many boots comp:%d",
                 pClient->component, 0,0,0,0,0);
         uint32_t boot_status = BS_BOOT_ATTEMPTS_EXCEEDED;
         bootloader_server_client_boot_err(pClient, boot_status);
     }
     if(!err) {
         pClient->fd = sh_data_open(pClient->component, O_RDONLY);
         if(pClient->fd < 0) {
             TraceE3(TrcMsgBLServ, "open err. fd:%d comp:%d",
                     pClient->fd, pClient->component,0,0,0,0);
             err = __LINE__;
         }
     }
     if(!err) {
         err = sh_data_stat(pClient->component, &buf);
         pClient->version = buf.version;
         pClient->size = buf.buf.st_size;
         pClient->crc  = buf.crc;
     }
     TraceDbg(TrcMsgBLServ, "err:%d fd:%d comp:%d size:%x",
              err, pClient->fd, pClient->component, buf.buf.st_size,0,0);
     if(!err) {
         /*
      * If first boot, version update, or possible corruption detected
      * send MCU a new image, else just boot it!
      * (or if we are forcing it boot with CONFIG_MCU_ALWAYS_BOOT,
      * send it anyway)
      */
         if(CONFIG_MCU_ALWAYS_BOOTLOAD ==  0 &&
            pClient->bl_info.msg.header_stat == BL_HDR_OK &&
            pClient->bl_info.msg.app_crc     == pClient->crc  &&
            pClient->bl_info.msg.app_version == pClient->version)
         {
             /* BOOT IT! */
             BL_RPLY_MSG(reply_msg, command_app_start);
             pClient->state = STATE_APP_START;
             cris_id |= CRIS_COMMAND_ID_START_APP;

         } else {
             /* LOAD IT! */
             pClient->state = STATE_WAITING_COM_OPEN;
             /* Open comms command */
             BL_RPLY_MSG(reply_msg, command_open_comms);
             /* This one message wants a node id. Not sure why? */
             pCmsg->data[CMD_OPEN_COMS_NODE_POS] = pCmsg->node_id;

         }
     }

     /* COMS Open -> Start Programming - Start with Erase */
 } else if(pClient->state == STATE_WAITING_COM_OPEN &&
           client_msg[BL_RPLY_INDEX_INIT_STATUS] == BL_RPLY_CLIENT_COM_OPENED) {

     pClient->state = STATE_PROG_START_ERASE;
     *((uint32_t*)&command_erase[BL_RPLY_INDEX_APP_VERSION]) = pClient->version;
     BL_RPLY_MSG(reply_msg, command_erase);
     cris_id |= CRIS_COMMAND_ID_PROG_START;

     /* Connection Closed by Client - the client should be running now */
 } else if(pClient->state == STATE_APP_START &&
           client_msg[BL_RPLY_INDEX_INIT_STATUS] == BL_RPLY_CLIENT_COM_CLOSED) {
     // All ok - App should have started.
     pClient->state = STATE_APP_SUCCESS;

     /* Any other time the connection closed, is an error */
 } else {
     TraceE3(TrcMsgBLServ, "Error state:%d comp:%d",
             pClient->state, pClient->component, 0,0,0,0);
     err = __LINE__;
 }
 break;
}

/* Blank Check & Data Read both have the same command ID */
case CRIS_COMMAND_ID_DISPLAY_DATA :
case CRIS_COMMAND_ID_PROG_START :
{
 /* Erase has finished, perform the blank check */
 if(pClient->state == STATE_PROG_START_ERASE) {
     pClient->state = STATE_PROG_START_BLANK_CHECK;
     BL_RPLY_MSG(reply_msg, command_blank_check);
     cris_id |= CRIS_COMMAND_ID_DISPLAY_DATA;

     /* Blank check Passed or Failed - if good start programming */
 } else if(pClient->state == STATE_PROG_START_BLANK_CHECK) {
     if(client_msg[0] != 0 || client_msg[1] != 0) {
         TraceE3(TrcMsgBLServ, "blank check err. comp:%d %x:%x",
                 pClient->component, client_msg[0],client_msg[1],0,0,0);
         // Any address in the first two fields indicates an error
         err = __LINE__;
     } else {
         pClient->state = STATE_PROG_DATA;
         BL_RPLY_MSG(reply_msg, command_prog_init);
         cris_id |= CRIS_COMMAND_ID_PROG_START;
     }

 } else if(pClient->state == STATE_PROG_DONE_READBACK_CHECK){
     if(client_msg[BL_RPLY_INDEX_READBACK_STATUS] ==
        BL_RPLY_CMD_OK_READBACK_CRC_FAILED){
         TraceE2(TrcMsgBLServ, "Readback CRC mismatch", 0, 0, 0,0,0,0);
         err = __LINE__;
     } else if(client_msg[BL_RPLY_INDEX_READBACK_STATUS] == BL_RPLY_READBACK_DONE) {

         uint32_t app_version_read =
             *((uint32_t*)&client_msg[BL_RPLY_INDEX_APP_NEW_VERSION]);

         uint16_t readback_crc =
             *((uint16_t*)&client_msg[BL_RPLY_INDEX_READBACK_CRC]);

         /* If nothing broke and the crcs check out, give the client the go-ahead
             to start it's app */
         if(!err                     &&
            (readback_crc == pClient->crc) &&
            (app_version_read == pClient->version))
         {
             BL_RPLY_MSG(reply_msg, command_app_start);
             pClient->state = STATE_APP_START;
             cris_id |= CRIS_COMMAND_ID_PROG_START;
         } else {
             TraceE2(TrcMsgBLServ, "Readback CRC mismatch or err during app start",
                     0, 0, 0,0,0,0);
             pClient->state = STATE_WAITING_INITIAL_BOOT;
             err = __LINE__;
         }
     }
 } else {
     TraceE3(TrcMsgBLServ, "Error state:%d comp:%d",
             pClient->state, pClient->component, 0,0,0,0);
     err = __LINE__;
 }
 /* All cases here place start in end address in the reply */
 if(!err && pClient->state != STATE_PROG_DONE_READBACK_CHECK &&
    pClient->state != STATE_APP_START) {
     stat_helper_t buf = {0};

     err = sh_data_stat(pClient->component, &buf);
     if(!err) {
         *((uint16_t*)&reply_msg[CRC_16_POS_UPPER]) = buf.crc;
     }

     reply_msg[CMD_FLASH_END_POS_UPPER] = (uint8_t)(pClient->size >> 8) & 0xFF;
     reply_msg[CMD_FLASH_END_POS_LOWER] = (uint8_t)(pClient->size & 0xFF);
 }
 break;
}

/* Start sending the binary data */
case CRIS_COMMAND_ID_PROG_DATA :
{
 /* Verify client is OK and send more data */
 if(!err && pClient->state == STATE_PROG_DATA) {
     if(client_msg[BL_RPLY_INDEX_PROG_DATA] != BL_RPLY_PROG_DATA_OK_MORE) {
         TraceE3(TrcMsgBLServ, "Prog not OK. comp:%d state:%d msg:%d",
                 pClient->component, pClient->state,
                 client_msg[BL_RPLY_INDEX_PROG_DATA],0,0,0);
         err = __LINE__;
     } else { /* READ data from (our) flash */
         err = storage_memory_read(pClient->fd, reply_msg, MSG_SIZE);
         if(err != MSG_SIZE) {
             TraceE3(TrcMsgBLServ, "Read incorrect bytes:%d",
                     err,0,0,0,0,0);
         } else {
             err = 0;
         }
         pClient->pos += MSG_SIZE; // FIXME - handle ragged ends
         if(pClient->pos >= pClient->size) {
             pClient->state = STATE_PROG_DATA_END;
         }
         cris_id |= CRIS_COMMAND_ID_PROG_DATA;
     }

     /* Verify program has ended properly */
 } else if(!err && pClient->state == STATE_PROG_DATA_END) {
     // Make sure client agrees we are done
     if(client_msg[BL_RPLY_INDEX_PROG_DATA] != BL_RPLY_PROG_DATA_OK_DONE) {
         TraceE3(TrcMsgBLServ, "Prog not OK. comp:%d state:%d msg:%d",
                 pClient->component, pClient->state,
                 client_msg[BL_RPLY_INDEX_PROG_DATA], 0,0,0);
         err = __LINE__;
     } else {
         if(pClient->bootloader_version >= READBACK_SUPPORT){
             /* Request readback crc*/
             *((uint16_t*)&command_read_check[3]) = pClient->size;
             BL_RPLY_MSG(reply_msg, command_read_check);
             pClient->state = STATE_PROG_DONE_READBACK_CHECK;
             cris_id |= CRIS_COMMAND_ID_DISPLAY_DATA;
         } else {
             // Readback not support - JUMP!
             BL_RPLY_MSG(reply_msg, command_app_start);
             pClient->state = STATE_APP_START;
             cris_id |= CRIS_COMMAND_ID_PROG_START;
         }
     }
 }
 break;
}
case CRIS_COMMAND_ID_START_APP:
 err = __LINE__; // Not implemented
 break;
case CRIS_COMMAND_ID_SELECT_MEM_PAGE :
 err = __LINE__; // Not implemented
 break;
default:
 TraceE3(TrcMsgBLServ, "Unhandled command comp:%d state:%d com:%d",
         pClient->component, pClient->state, pCmsg->command,0,0,0);
 err = __LINE__; // FIXME - Close and reset
}

/* Check for errors & reply to client */
if(!err) {
/* If the app started go back to waiting for a new boot */
if(pClient->state == STATE_APP_SUCCESS) {
 pClient->size = 0;
 pClient->pos = 0;
 pClient->state = STATE_WAITING_INITIAL_BOOT;
 if(pClient->fd >= 0) {
     storage_memory_close(pClient->fd);
     pClient->fd = -1;
 }
 /* IF the app isn't done the client message should be filled in - send it*/
} else {
 pCmsg->pMessage->bootloader_callback(cris_id, pCmsg->data, CAN_DLC_8);
}

/* Handle errors */
} else {
TraceE3(TrcMsgBLServ, "Error comp:%d state:%d com:%d",
     pClient->component, pClient->state, pCmsg->command,0,0,0);
pClient->last_error = err;
if(pClient->fd >= 0) {
 storage_memory_close(pClient->fd);
 pClient->fd = -1;
}
uint32_t boot_status = BS_BOOT_ERROR_GENERAL;
bootloader_server_client_boot_err(pClient, boot_status);
}
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
static void bootloader_server_task(void *pvParameters)
{
    int err = 0;
    BaseType_t xStatus = pdPASS;
    bootload_msg_t message;
    TickType_t delay = BOOTLOADER_INITIAL_DELAY;

    while (1) {
        xStatus = xQueueReceive(xBLQueue, &message, delay);
        if (xStatus == pdFALSE) {
         // TODO handle a timeout waiting for clients
        } else {
                client_message_t cmsg;
                cmsg.pMessage = &message;
                cmsg.command  = message.cris_id & ~CRIS_COMMAND_MASK;
                cmsg.client_id = message.cris_id & CRIS_COMMAND_MASK;
                memset(cmsg.data, 0, sizeof(cmsg.data));
                BootLoaderClient_t *pClient = NULL;

                /*
                * Subtract 1 from all component Ids so they line up in
                * client data structure.  Need to find a better way.
                */
                switch (cmsg.client_id) {
                case BL_CRIS_ID_KEEPER:
                    pClient = &clients[COMPONENT_ID_TO_CLIENT_INDEX(COMPONENT_KEEPER)];
                    cmsg.node_id = BL_NODE_ID_KEEPER;
                    break;
                case BL_CRIS_ID_ANODE:
                    pClient = &clients[COMPONENT_ID_TO_CLIENT_INDEX(COMPONENT_ANODE)];
                    cmsg.node_id = BL_NODE_ID_ANODE;
                    break;
                case BL_CRIS_ID_MAGNET_O:
                    pClient = &clients[COMPONENT_ID_TO_CLIENT_INDEX(COMPONENT_MAGNET_0)];
                    cmsg.node_id = BL_NODE_ID_MAGNET_O;
                    break;
                case BL_CRIS_ID_MAGNET_I:
                    pClient = &clients[COMPONENT_ID_TO_CLIENT_INDEX(COMPONENT_MAGNET_1)];
                    cmsg.node_id = BL_NODE_ID_MAGNET_I;
                    break;
                case BL_CRIS_ID_VALVES:
                    pClient = &clients[COMPONENT_ID_TO_CLIENT_INDEX(COMPONENT_VALVES)];
                    cmsg.node_id = BL_NODE_ID_VALVES;
                    break;
                default:
                     err = __LINE__;
                }

                if(!err) {
                    bootloader_service_client(pClient, &cmsg);
                }
        }
    }
}
/**
* Callback function for process bootloader messages.  Packages up the message
* and places it in the bootloader queue.
*
* @param id  Message ID from client
* @param data pointer to message data
* @param dlc data length code
* @return 0
*/
int bootloader_server_callback(message_t* msg)
{
    int err = 0;
    bootload_msg_t message;

    message.cris_id = msg->id;
    message.bootloader_callback = mcan_send_msg_client;
    memcpy(message.data, msg->data, msg->dlc);
    xQueueSend(xBLQueue, &message, 0);

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
int bootloader_server_init(void)
{
    /* Create the queues */
    xBLQueue = xQueueCreateStatic( BUFFER_COUNT, QUEUE_ITEM_SIZE,
                           ucBLQueueStorageArea, &xBLStaticQueue );

    /* Create File Transfer Tasks */
    xTaskCreateStatic(bootloader_server_task, "Bootloader Server Task",
               BL_TASK_STACK_SIZE, 0, BL_TASK_PRIORITY, blStack, &blTaskBuffer);

    mcan_task_register_cb(&bootloader_node);

return 0;
}

/* *****************************************************************************
End of File
*/
