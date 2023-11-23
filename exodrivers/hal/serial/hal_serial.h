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

#ifndef THRUSTER_CONTROL_HALO12_VA41630_KRYPTON_SILVER_BCT_EXODRIVERS_HAL_SERIAL_HAL_SERIAL_H_
#define THRUSTER_CONTROL_HALO12_VA41630_KRYPTON_SILVER_BCT_EXODRIVERS_HAL_SERIAL_HAL_SERIAL_H_
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hal.h"
#ifdef FREE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */
typedef struct _serial_frame {
  uint16_t sof;
  uint8_t dlc;
  uint8_t data[8]; // data plus DLC
  union {
    uint16_t crc;
    uint8_t bytes[2];
  };
} serial_frame_t;
#pragma pack(pop)

/* UART_IF_ID_X's correspond to UART numbers */
typedef enum {
  UART_IF_ERR = -1, // UART ID error
  UART_IF_ID_0,     // UART 0
  UART_IF_ID_1,     // UART 1
  UART_IF_ID_2,     // UART 2
  UART_IF_ID_EOL
} uart_if_id_t;

/* Baud in bits per second */
#define UART_BAUD_9600   9600
#define UART_BAUD_57600  57600
#define UART_BAUD_115200 115200
#define UART_BAUD_230400 230400

/* Structure used to init UART */
typedef struct uart_init {
  uart_if_id_t uart_if_id;
  unsigned int baud;
  bool rx_irq_enable;
  bool tx_irq_enable;
  uint32_t tx_irq_priority;
  uint32_t rx_irq_priority;
  uint32_t tx_interrupt_level;      //Length of message to trigger interrupt
  uint32_t rx_interrupt_level;      //Length of message to trigger interrupt
#ifdef FREE_RTOS
  uint32_t rx_task_prio;            //rx task priority
  uint32_t tx_task_prio;            //tx task priority
#endif
} uart_init_t;

/**
 * Init UART
 *  NOTE: This init assumes a lot:
 *               no parity
 *               1 stop bit
 *               8 bit word
 *               no loopback
 *               no flowcontrol
 *
 * @param ui - uart parameters to use for init
 *
 * @return handle to uart or -1 on err
 */
int uart_init(const uart_init_t *ui);

/**
 * Write UART bytes without DMA
 *
 * @param handle - handle to UART
 * @param tx_buf - buffer to transmit over UART
 * @param size   - number of bytes to transmit
 *
 * @return 0 on success or -1 on err
 */
int uart_write_raw(const int handle, uint8_t *tx_buf, const size_t size);

/**
 * Write UART bytes with DMA
 *
 * @param handle - handle to UART
 * @param tx_buf - buffer to transmit over UART
 * @param size   - number of bytes to transmit
 *
 * @return 0 on success or -1 on err
 */
int uart_write(int handle, message_t *msg, int timeout);

int uart_read(int handle, message_t *msg, int timeout);

#endif//THRUSTER_CONTROL_HALO12_VA41630_KRYPTON_SILVER_BCT_EXODRIVERS_HAL_SERIAL_HAL_SERIAL_H_
