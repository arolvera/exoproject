#ifndef _CAN_H
#define _CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#ifdef FREE_RTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif



typedef enum{
	/* Fill in as needed */
	CAN_BAUD_RATE_1000,
} can_baud_rate_t;

typedef enum{
	CAN_FILTER_ID,
	CAN_FILTER_RANGE,
} can_filter_t;

typedef struct can_rx_buffer_t_{
	const can_filter_t filter_type;      //either CAN_FILTER_ID or CAN_FILTER_RANGE
	const uint32_t filter_high;
	const uint32_t filter_low;
	const uint32_t rx_mob_count;            //specifies number of mobs to use for a filter
} can_rx_buffer_t;

typedef struct can_init_t_{
	const can_baud_rate_t baud;
	int tx_mob_count;
	can_rx_buffer_t* rx_buffers;         //List of can_rx_buffers
    int rx_buffer_len;
	uint32_t irq_priority;
    uint32_t task_priority;
#ifdef FREE_RTOS
    //User can give a queue handle for received messages to be sent to.
    //If NULL rx queue is set by internal can implementation
    QueueHandle_t *rx_q_handle;
#endif
} can_init_t;

/** 
 * Initialize the CAN driver 
 *  
 * @param  
 * @param can_init structure containing init parameters 
 *   
 * @return 0 on success or -1 on err
 */
int can_init(can_init_t* can_init);

/**
 * Send a CAN message 
 *  
 * @param handle handle to the CAN interface 
 * @param id CAN MOb ID 
 * @param data pointer to data buffer 
 * @param dlc data length code
 * @param timeout timeout for queueing 
 *   
 * @return 0 on success or !0 on err
 */
uint32_t can_send(const int handle, message_t* msg, const uint32_t timeout);


/** 
 * Get the next message out of the queue 
 *  
 * @param handle handle to the CAN interface 
 * @param q_item pointer to dequeue can messages into  
 * @param timeout timeout for queueing 
 *   
 * @return 0 on success or !0 on err
 */
uint32_t can_rcv(int handle, message_t* q_item, int timeout);


/* FOR USE IN BARE_METAL APPLICATIONS ONLY */
/** 
 * Use to check whether or not CAN buffers are ready for RX processing 
 *  
 * @param handle handle to the CAN interface 
 *   
 * @return void 
 */
void can_rx_service(const int handle);


/* FOR USE IN BARE_METAL APPLICATIONS ONLY */
/** 
 * Check if more messages are queue'd and available for processing 
 *    
 * @param handle handle to the CAN interface 
 *   
 * @return void 
 */
uint32_t can_rx_is_empty(int handle);
#ifdef __cplusplus
}
#endif

#endif