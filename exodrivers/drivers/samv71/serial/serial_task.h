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
/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _SERIAL_TASK_H    /* Guard against multiple inclusion */
#define _SERIAL_TASK_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "FreeRTOS.h"                   // RTOS Definitions
#include "queue.h"                      // RTOS Queue Defines
#include "error/error_handler.h"


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif
    
/* Which serial device to use - older boards used the UART, which is easier
 * on the dev boards.   Newer (flight) boards use a USART to take advantage of
 * the RS-485 mode */
typedef enum {
    SERIAL_UART0,
    SERIAL_USART1,
    SERIAL_USART2,
} serial_device_t;

/* Debug Variables - Global for CO OD entry */
typedef struct serial_debug {
    int serial_rx_interrupts;
    int serial_rx_messages;
    int serial_tx_interrupts;
    int serial_tx_messages;
    int error_ticks;
    bool has_overrun_err;
    bool has_crc_err;
    bool has_frame_err;
} serial_debug_t;

#define SERIAL_HAS_COM_ERROR(__X__) (__X__.has_overrun_err ||  \
                                     __X__.has_crc_err     ||  \
                                     __X__.has_frame_err       \
                                     )

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */

typedef struct serial_error_detail{
    base_error_detail_t b_d;
} serial_error_detail_t;    

#pragma pack(pop)                  /* restore original alignment from stack   */

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/**
 * 
 * @Function
 *    int serial_task_init(void);
 * 
 * @Summary
 *     Initialize serial tasks.
 * 
 * @Description
 *  Create two tasks, one for receive and one for transmit.  Data is DMA'd from
 *  the associated serial device.
 * 
 * @Precondition
 *     None
 * 
 * @Returns 0 on success
 * 
 */
int  serial_task_init(serial_device_t *dev);
int  serial_task_resync_status_get(void);
void serial_task_resync_status_reset(void);
int  serial_ext_msg_rcv(uint32_t *identifier, uint8_t *data, uint8_t *dlc, int block);

void serial_start_rx(void);
void serial_task_send(uint16_t cobid, uint8_t *data, uint8_t dlc);
void serial_task_SDO_queue_set(QueueHandle_t *pxOQueue);
void serial_ext_msg_snd(uint32_t identifier, uint8_t *data, uint8_t dlc);
bool serial_has_err(void);
void serial_error_handler_init(void);

volatile serial_debug_t *serial_debug_get(void);

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _SERIAL_TASK_H */

/* *****************************************************************************
 End of File
 */
