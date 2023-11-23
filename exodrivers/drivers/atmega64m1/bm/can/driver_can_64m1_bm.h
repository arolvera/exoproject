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
#ifndef _ATMEGA64M1_CAN_DRIVER_H
#define _ATMEGA64M1_CAN_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "mcu_include.h"

#define MAX_MOBS 6
#define RESET 0x00
#define CAN_REV_A false
#define CAN_REV_B true
#define MAX_DATA_LENGTH 8
#define RX_DATA_READY_TO_BE_READ true
#define RX_DATA_READ false

typedef enum
{
	CAN_BAUD_RATE_100  = 100,
	CAN_BAUD_RATE_125  = 125,
	CAN_BAUD_RATE_200  = 200,
	CAN_BAUD_RATE_250  = 250,
	CAN_BAUD_RATE_500  = 500,
	CAN_BAUD_RATE_1000 = 1000,
} BaudRate_t;

typedef enum
{
	SUCCESS = 0,
	CAN_INIT_INVALID_BAUD_RATE,
	CAN_INIT_INTERRUPT_ENABLES_STRUCT_NULL,
	MOB_INIT_NULL_ARG,
	MOB_INIT_INVALID_MOB_NUM,
	MOB_INIT_RETURN_CODES_SIZE,
	MOB_INIT_INVALID_MOB_CONFIGURATION,
	MOB_INIT_INVALID_DATA_LENGTH,
	MOB_INIT_INVALID_DEVICE_ID_TAG,
	MOB_INIT_INVALID_DEVICE_ID_MASK,
    MOB_INIT_NO_FREE_MOBS,
	CAN_INIT_RETURN_CODES_SIZE
}initReturnCodesEnum_t;

/**
 * CAN MSG Callback function. 
 * 
 * The FIFO pointer points to a register that can be read in a loop, the index
 * is auto-incremented every time the data is read from pointer.  When
 * implementing this function, read from the fifo pointer 'dlc' times.  Do not
 * increment the pointer.
 * 
 * For example:
 * uint8_t buffer[8] = {};
 * for(uint8_t i = 0; i < dlc; i++) {
 *    buffer[i] = *fifo; // DO NOT increment fifo!
 * }
 * 
 * @param id Received CAN Message ID
 * @param dlc Data Length Code
 * @param fifo CAN data buffer message FIFO
 */
typedef void (*CAN_ISR_CALLBACK) (uint32_t id, uint8_t dlc, volatile uint8_t *fifo);

uint8_t canControllerInit(bool ttcEnable, bool ttcSync, uint32_t baud);
bool canSendMsg(uint32_t id, uint8_t *txbuffer);
int can_send_msg(uint32_t id, int dlc, uint8_t *data);
uint16_t timestampGet(void);
bool command_rx(uint8_t* rxBuffer);
initReturnCodesEnum_t canMobInitRx(uint8_t count, uint32_t id, uint32_t mask, CAN_ISR_CALLBACK cb);
uint16_t can_err_code_get(void);
health_table_entry_t *can_health_get(void);

#endif
