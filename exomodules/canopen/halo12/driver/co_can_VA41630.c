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

#include "co_can_VA41630.h"
#include <string.h>
#include "definitions.h"

static void DrvCanInit(void);
static void DrvCanEnable(uint32_t baudrate);
static int16_t DrvCanSend(CO_IF_FRM *frm);
static int16_t DrvCanRead(CO_IF_FRM *frm);
static void DrvCanReset(void);
static void DrvCanClose(void);

const CO_IF_CAN_DRV CanOpenDriver = {
    DrvCanInit,
    DrvCanEnable,
    DrvCanRead,
    DrvCanSend,
    DrvCanReset,
    DrvCanClose
};

static int ext_uart_handle;

static void DrvCanInit(void)
{
    ext_uart_handle = uart_init(&ext_uart_init);
}

static void DrvCanEnable(uint32_t baudrate)
{
    /* TODO: set the given baudrate to the CAN controller */
}

static int16_t DrvCanSend(CO_IF_FRM *frm)
{

    return uart_write(ext_uart_handle, (message_t *)frm, portMAX_DELAY);
//    return (0u);
}

/**
 * This driver function is called when the CANopen node processing is started.
 * This function is intended to receive all messages from the CAN network.
 * @param frm pointer to store message info
 * @return The function returns the number processed bytes sizeof(CO_IF_FRM) on
 *          success, (int16_t)0 in case of no reception, or (int16_t)-1 when an
 *          error is detected.
 */
//typedef struct CO_IF_FRM_T {         /*!< Type, which represents a CAN frame */
//  uint32_t  Identifier;            /*!< CAN message identifier             */
//  uint8_t   Data[8];               /*!< CAN message Data (payload)         */
//  uint8_t   DLC;                   /*!< CAN message data length code (DLC) */
//} CO_IF_FRM;

static int16_t DrvCanRead(CO_IF_FRM *frm)
{
    int16_t err = 0;
    message_t msg;
    int rcv_err = uart_read(ext_uart_handle, &msg, portMAX_DELAY);
    if(!rcv_err) {
        //convert to CO_IF_FRM
        frm->DLC = msg.dlc;
        memcpy(frm->Data, msg.data, msg.dlc);
        frm->Identifier = msg.id;
    }
    if(!rcv_err) {
        /* -1 is an error, zero means no message, 1 is message, return size of message */
        err = sizeof(CO_IF_FRM);
    }

    return err;
}

static void DrvCanReset(void)
{
    /* TODO: reset CAN controller while keeping baudrate */
}

static void DrvCanClose(void)
{
    /* TODO: remove CAN controller from CAN network */
}
