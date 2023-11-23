#ifndef _CAN_VORAGO_DRIVER_H
#define _CAN_VORAGO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can/hal_can.h"
#include "device.h"

#define VORAGO_TOTAL_BUFFERS (sizeof(can_0_buffers)/sizeof(can_0_buffers[0]))
#define CAN_0_IF_NUM 0
#define CAN_1_IF_NUM 1

typedef struct _can_cmb_t {
  volatile uint32_t  CNSTAT; /*!< Buffer Status / Control Register */
  volatile uint32_t  TSTP;   /*!< CAN Frame Timestamp              */
  volatile uint32_t  DATA3;  /*!< CAN Frame Data Word 3[15:0]      */
  volatile uint32_t  DATA2;  /*!< CAN Frame Data Word 2[15:0]      */
  volatile uint32_t  DATA1;  /*!< CAN Frame Data Word 1[15:0]      */
  volatile uint32_t  DATA0;  /*!< CAN Frame Data Word 0[15:0]      */
  volatile uint32_t  ID0;    /*!< CAN Frame Identifier Word 0      */
  volatile uint32_t  ID1;    /*!< CAN Frame Identifier Word 1      */
} can_cmb_t;


/** 
 * Process a CAN message 
 *   
 * @param handle handle to the CAN interface 
 * @param num_msgs_to_process number of messages to process  
 *   
 * @return 0 on success or !0 on err
 */
int can_driver_process_rx(const int handle, const unsigned int num_msgs_to_process);


/** 
 * Get the mask with bits corresponding to which buffers are configured
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return 0 on success or !0 on err
 */
uint32_t rx_buffer_mask_get(int handle);


/** 
 * Get the mask with bits corresponding to which buffers are configured
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return pointer to CAN buffer for transmitting or receiving 
 */
can_cmb_t* can_buffer_get(int handle);


/** 
 * Init the CAN driver 
 *    
 * @param can_init init structure for CAN driver
 *   
 * @return 0 on success, !0 on failure 
 */
int can_driver_init(can_init_t* can_init);


/** 
 * Transmit a CAN message 
 *    
 * @param can_cmb the CAN message to send 
 *   
 * @return 0 on success, !0 on failure 
 */
int can_driver_tx(volatile can_cmb_t* can_cmb);


/** 
 * Find which buffers are ready to have their data copied 
 *    
 * @param handle handle to the CAN interface 
 *     
 * @return 0 on success, !0 on failure 
 */
uint32_t rx_data_buff_ready_get(const int handle);


/** 
 * Reset the flags that indicate which buffers are ready for receiving 
 *      
 * @param handle handle to the CAN interface
 *   
 * @return 0 on success, !0 on failure 
 */
void rx_data_buff_ready_reset(const int handle);


/** 
 * Reset the flags that indicate which buffers are ready for receiving 
 *      
 * @param handle handle to the CAN interface
 *   
 * @return 0 on success, !0 on failure 
 */
const char* can_if_handle_get(const int if_num);


/* Queue operations */
/* NOTE: ONLY FOR USE IN BARE METAL IMPLEMENTATIONS */
void enqueue(can_cmb_t* q_item);
void enqueue_overflow(can_cmb_t* q_item);
int queue_empty(void);



#ifdef __cplusplus
}
#endif

#endif

