#ifndef _UART_DRIVER_VORAGO_H
#define _UART_DRIVER_VORAGO_H


#include <stdbool.h>
#include <stdint.h> 
#include <stddef.h>
#include "serial/hal_serial.h"

/** 
 * Get address of DATA pointer (useful for setting up DMA)
 *
 * @param handle - handle to uart 
 * 
 * @return pointer to UART DATA pointer of NULL on error 
 */
volatile uint32_t* uart_data_ptr_get(const uart_if_id_t handle);


/** 
 * Disable UART RX interrupts 
 *  
 * @param handle - handle to uart to disable RX interrupts of 
 * 
 * @return 0 on success, !0 on err 
 */
int uart_rx_interrupt_disable(const uart_if_id_t handle);


/** 
 * Enable UART RX interrupts 
 *  
 * @param handle - handle to uart to disable RX interrupts of 
 * 
 * @return 0 on success, !0 on err 
 */
int uart_rx_interrupt_enable(const uart_if_id_t handle);


/** 
 * Set FIFO fill level that will trigger an interrupt
 *  
 * @param handle - handle to uart to set FIFO level of
 * @param trg - FIFO level to triger an interrupt

 * 
 * @return 0 on success, !0 on err
 */
int uart_fifo_irq_trg_set(const uart_if_id_t handle, int trg);

/**
 * Check the Uart's TX status
 *
 * @param uart_handle - handle to uart's ops
 *
 * @return uart TX status.  -1 if handle invalide
 */
int uart_tx_status_get(int uart_handle);

#endif

