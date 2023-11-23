#include "driver_can_va41630_rtos.h"
#include "can/driver_can_va41630_cmn.h"
#include "osal/osal.h"

#define CAN_TASK_STACK_SIZE   (configMINIMAL_STACK_SIZE)

#define BUFFER_COUNT_TX    (16 * 2)
#define BUFFER_COUNT_RX    (16 * 2)

static const char can_0_rx_task_name[] = "CAN0Rx";
static const char can_1_rx_task_name[] = "CAN1Rx";
static const char can_0_tx_task_name[] = "CAN0Tx";
static const char can_1_tx_task_name[] = "CAN1Tx";


#define QUEUE_ITEM_SIZE_TX (sizeof(message_t))
#define QUEUE_ITEM_SIZE_RX (sizeof(message_t))
#define CAN_QUEUE_TX_STATIC_SIZE (QUEUE_ITEM_SIZE_TX * BUFFER_COUNT_TX)
#define CAN_QUEUE_RX_STATIC_SIZE (QUEUE_ITEM_SIZE_RX * BUFFER_COUNT_RX)

typedef struct can_rtos_ops{
	TaskHandle_t  can_rx_task_handle;
	const char*   can_rx_task_name;
	TaskHandle_t  can_tx_task_handle;
	const char*   can_tx_task_name;
	const int     can_if_num;
	uint32_t timeout_ms;
	uint32_t num_msgs_to_process;
	QueueHandle_t *can_if_rx_q_handle;
	uint8_t       ucRxQueueStorageArea[CAN_QUEUE_RX_STATIC_SIZE];
	StaticQueue_t xRxStaticQueue;
	uint32_t rx_q_item_size;
	QueueHandle_t can_if_tx_q_handle;
	uint8_t       ucTxQueueStorageArea[CAN_QUEUE_TX_STATIC_SIZE];
	StaticQueue_t xTxStaticQueue;
	uint32_t tx_q_item_size;
	StackType_t   rx_task_stack[CAN_TASK_STACK_SIZE];
	StaticTask_t  rx_task_buffer;
	uint32_t buffer_count_rx;
	StackType_t   tx_task_stack[CAN_TASK_STACK_SIZE];
	StaticTask_t  tx_task_buffer;
    uint32_t buffer_count_tx;
    SemaphoreHandle_t tx_stack_sem;
    StaticSemaphore_t txSemaphoreBuffer;
} can_rtos_ops_t;

static can_rtos_ops_t can_rtos_ops[] = {{.can_rx_task_name = can_0_rx_task_name, .can_tx_task_name = can_0_tx_task_name, 
                                         .can_if_num = CAN_0_IF_NUM,             .timeout_ms = 10, 
                                         .num_msgs_to_process = 10,              .rx_q_item_size = QUEUE_ITEM_SIZE_RX,  
										 .tx_q_item_size = QUEUE_ITEM_SIZE_TX,   .buffer_count_rx = BUFFER_COUNT_RX,
										 .buffer_count_tx = BUFFER_COUNT_TX},

										{.can_rx_task_name = can_1_rx_task_name, .can_tx_task_name = can_1_tx_task_name,
										 .can_if_num = CAN_1_IF_NUM,             .timeout_ms = 10, 
										 .num_msgs_to_process = 10,              .rx_q_item_size = QUEUE_ITEM_SIZE_RX,  
										 .tx_q_item_size = QUEUE_ITEM_SIZE_TX,   .buffer_count_rx = BUFFER_COUNT_RX,
										 .buffer_count_tx = BUFFER_COUNT_TX}};

#define CAN_RTOS_OPS_SIZE (sizeof(can_rtos_ops) / sizeof(can_rtos_ops[0]))

/** 
 * Rx Task function.  Wake up when the interrupt notifies and try to process RX buffers 
 *   
 * @param arg can_rtos_ops_t* that contains interface parameters 
 *   
 * @return void 
 */
static void can_rx_task(void* arg)
{
	can_rtos_ops_t* rtos_ops = (can_rtos_ops_t*)arg;

	for (;;) {
		/* Wait for notification that RX message is available. */
		uint32_t notified = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(notified){
			can_driver_process_rx(rtos_ops->can_if_num, rtos_ops->num_msgs_to_process);
		}
	}
}


/** 
 * Tx Task function.  
 *   
 * @param arg can_rtos_ops_t* that contains interface parameters 
 *   
 * @return void 
 */
static void can_tx_task( void* arg)
{
	BaseType_t xStatus = pdPASS;

	can_rtos_ops_t* rtos_ops = (can_rtos_ops_t*)arg;

	for(;;){
        volatile int err = 0;
        message_t msg = {0};
        xStatus = xQueueReceive(rtos_ops->can_if_tx_q_handle, &msg, portMAX_DELAY);
        if(xStatus) {
            xStatus = xSemaphoreTake(rtos_ops->tx_stack_sem, portMAX_DELAY);

            if(!xStatus) {
                err = __LINE__;
            }
        }

        if(xStatus && !err) {
            volatile can_cmb_t *can_cmb = can_buffer_get(rtos_ops->can_if_num);
            if(can_cmb != 0) {
                can_cmb->ID1 = msg.id << 5;
                can_cmb->CNSTAT = (msg.dlc << CAN_CNSTAT_CMB0_DLC_Pos) & CAN_CNSTAT_CMB0_DLC_Msk;
                can_cmb->DATA0 = (msg.data[1] | (msg.data[0] << 8));
                can_cmb->DATA1 = (msg.data[3] | (msg.data[2] << 8));
                can_cmb->DATA2 = (msg.data[5] | (msg.data[4] << 8));
                can_cmb->DATA3 = (msg.data[7] | (msg.data[6] << 8));
                can_driver_tx(can_cmb);

            } else {
                err = __LINE__;
            }
        }
    }
}


/** 
 * Init all things FreeRTOS
 *     
 * @param handle handle to interface 
 * @param num_tx_buffers parameter to use when setting up TX semaphores 
 *   
 * @return 0 on success !0 on failure 
 */
static int can_init_rtos(const int handle, can_init_t *can_init)
{
	static uint32_t if_num = 0;
	int err = 0;
	if(if_num < 2 && handle < (int)CAN_RTOS_OPS_SIZE){
		can_rtos_ops_t* c = &can_rtos_ops[handle];
//        if(can_init->rx_q_handle == NULL) {
//            c->can_if_rx_q_handle = xQueueCreateStatic(BUFFER_COUNT_RX, QUEUE_ITEM_SIZE_RX, c->ucRxQueueStorageArea,
//                                                       &c->xRxStaticQueue);
//        }else{
        c->can_if_rx_q_handle = can_init->rx_q_handle;
//        }
		c->can_rx_task_handle = xTaskCreateStatic(can_rx_task, 
													c->can_rx_task_name,
													sizeof(c->rx_task_stack) / sizeof(c->rx_task_stack[0]), 
													(void*)c, 
													can_init->task_priority,
													c->rx_task_stack, 
													&c->rx_task_buffer);

		if(can_init->tx_mob_count > 0){
            OSAL_SEM_CreateStatic(&c->tx_stack_sem, OSAL_SEM_TYPE_COUNTING, can_init->tx_mob_count,
                                  can_init->tx_mob_count, &c->txSemaphoreBuffer, "tx_stack");
			OSAL_QUEUE_CreateStatic(&c->can_if_tx_q_handle,c->buffer_count_tx,
                                  c->tx_q_item_size, c->ucTxQueueStorageArea,
                                  &c->xTxStaticQueue, "CAN_TXq");
			c->can_tx_task_handle = xTaskCreateStatic(can_tx_task,
														c->can_tx_task_name,
														sizeof(c->tx_task_stack) / sizeof(c->tx_task_stack[0]),												
														(void*)c, 
														can_init->task_priority,
														c->tx_task_stack, 
														&c->tx_task_buffer);

		} else {
			err = __LINE__;
		}
	}
	
	return err;
}


/** 
 * Put message info FreeRTOS queue 
 *     
 * @param handle handle to interface 
 * @param msg message to enqueue 
 *    
 * @return 0 on success !0 on failure 
 */
int can_queue_rx_message_RTOS(int handle, message_t* msg)
{
	int err = 0;
	if(handle >= 0 && handle < (int)CAN_RTOS_OPS_SIZE){
        uint16_t swap_data[4] = {0};

        swap_data[0] = (msg->data[1] | (msg->data[0] << 8));
        swap_data[1] = (msg->data[3] | (msg->data[2] << 8));
        swap_data[2] = (msg->data[5] | (msg->data[4] << 8));
        swap_data[3] = (msg->data[7] | (msg->data[6] << 8));
        memcpy(msg->data, swap_data, sizeof(swap_data));
		BaseType_t xStatus = xQueueSend(*can_rtos_ops[handle].can_if_rx_q_handle, msg, portMAX_DELAY);
		if(!xStatus){
			err = __LINE__;
		}
	} else {
		err = __LINE__;
	}

	return err;
}


/** 
 * Notify application that a buffer is free for transmitting
 *      
 * @param arg cast to handle of interface 
 *    
 * @return xHigherPriorityTaskWoken 
 */
int can_notif_tx_buff_avail_RTOS(void* arg)
{
	int handle = *((int*)arg);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(handle >= 0 && handle < (int)CAN_RTOS_OPS_SIZE){
		xSemaphoreGiveFromISR(can_rtos_ops[handle].tx_stack_sem, &xHigherPriorityTaskWoken);
	}

	return xHigherPriorityTaskWoken;
}


/** 
 * Enqueue a CAN message for sending
 *        
 * @param handle handle of interface 
 * @param msg message to enqueue  
 * @param timeout timeout to wait for queue operations 
 *  
 * @return 0 on success !0 on failure 
 */
uint32_t can_send(const int handle, message_t* msg, const uint32_t timeout)
{
	int err = 0;
	if(msg != NULL && handle >= 0 && handle < (int)CAN_RTOS_OPS_SIZE){

		BaseType_t xStatus = xQueueSend(can_rtos_ops[handle].can_if_tx_q_handle, msg, timeout);
		if(!xStatus){
			err = __LINE__;
		}
	} else {
		err = __LINE__;
	}

	return err;
}


/** 
 * Receive a CAN message 
 *        
 * @param handle handle of interface 
 * @param msg message pointer to receive in to   
 * @param timeout timeout to wait for queue operations 
 *  
 * @return 1 on success 0 on failure
 */
uint32_t can_rcv(int handle, message_t *msg, int timeout)
{
	BaseType_t status = pdTRUE;
	if(msg != 0 && (handle >= 0 && handle < (int)CAN_RTOS_OPS_SIZE)){
 		status = xQueueReceive(*can_rtos_ops[handle].can_if_rx_q_handle, msg, timeout);
	}

	return status;
}													 


/** 
 * Enqueue a CAN message for sending
 *        
 * @param arg cast to handle of interface 
 *  
 * @return xHigherPriorityTaskWoken 
 */
int can_notify_rx_task_RTOS(void* arg)
{
	int handle = *((int*)arg);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(handle >= 0 && handle < (int)CAN_RTOS_OPS_SIZE){
		vTaskNotifyGiveFromISR(can_rtos_ops[handle].can_rx_task_handle, &xHigherPriorityTaskWoken);
	}

	return xHigherPriorityTaskWoken;
}


/** 
 * Init the CAN driver 
 *        
 * @param can_init structure of init params 
 *  
 * @return 0 on success, -1 on failure 
 */
int can_init(can_init_t* can_init)
{
	int handle = -1;
	int err = 0;
	if(can_init != 0){
		err = can_driver_init(can_init);
		if(err != -1){
			handle = err;
			err = can_init_rtos(handle, can_init);
		}
	} else {
		err = -1;
	}

	return err == -1 ? err : handle;
}



bool can_get_next_message(int handle, can_cmb_t *msg, int timeout)
{
    return !xQueueReceive(*can_rtos_ops[handle].can_if_rx_q_handle, msg, timeout);
}




