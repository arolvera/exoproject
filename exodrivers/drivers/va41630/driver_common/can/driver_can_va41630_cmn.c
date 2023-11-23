#include "driver_can_va41630_cmn.h"

/* adjust per can device */
#define VORAGO_CAN_INTERFACE_NUM 2

#include "utils/stack.h"

#define ENABLE_ALL_INTERRUPTS 0xFFFF

typedef int (*NOTIFY_CB)(void* arg);
typedef int (*Q_MSG_CB)(int handle, message_t* msg);

typedef struct driver_ops{
	NOTIFY_CB notify_tx_buff_avail_impl;
	NOTIFY_CB notify_task_impl;
	Q_MSG_CB  queue_message_impl;
}driver_ops_t;

#ifdef FREE_RTOS
    #include "can/driver_can_va41630_rtos.h"
static driver_ops_t ops = {.notify_tx_buff_avail_impl = can_notif_tx_buff_avail_RTOS,
							   .notify_task_impl          = can_notify_rx_task_RTOS,
							   .queue_message_impl        = can_queue_rx_message_RTOS};
	/* FreeRTOS irq priorities need to br greater than or equal to a default irq value, in our case this value is 5 */
	#define IRQ_PRIORITY_SET(__X__) (5 + __X__)
#else
	#include "can/driver_can_va41630_bm.h"
	static driver_ops_t ops = {.notify_tx_buff_avail_impl = can_notif_tx_buff_avail_BM,
							   .queue_message_impl        = can_queue_rx_message_BM};
	#define IRQ_PRIORITY_SET(__X__) (__X__)
	#define portYIELD_FROM_ISR(__X__) 
#endif

typedef enum {
  en_can_cmb_cnstat_st_RX_NOT_ACTIVE = 0x0, //CNSTAT_ST must be set to this to enable writes to rx ID filters
  en_can_cmb_cnstat_st_RX_READY      = 0x2, //code will set CNSTAT_ST to this to enable rx
  en_can_cmb_cnstat_st_RX_BUSY0      = 0x3,
  en_can_cmb_cnstat_st_RX_FULL       = 0x4, //rx buf is valid
  en_can_cmb_cnstat_st_RX_BUSY1      = 0x5,
  en_can_cmb_cnstat_st_RX_OVERRUN    = 0x6, //rx buf is valid & latest but at least one earlier rx was tossed, NOT used if BUFLOCK
  en_can_cmb_cnstat_st_RX_BUSY2      = 0x7,
  en_can_cmb_cnstat_st_TX_NOT_ACTIVE = 0x8, //code can set CNSTAT_ST to this to load tx or abort a stuck tx  
  en_can_cmb_cnstat_st_TX_RTR        = 0xa, //code sets CNSTAT_ST to this for automated CMB *response* to a matching RTR
  en_can_cmb_cnstat_st_TX_ONCE       = 0xc, //code sets CNSTAT_ST to this to transmit RTR or non-RTR
  en_can_cmb_cnstat_st_TX_BUSY0      = 0xd,
  en_can_cmb_cnstat_st_TX_ONCE_RTR   = 0xe,
  en_can_cmb_cnstat_st_TX_BUSY2      = 0xf
} en_can_cmb_cnstat_st_t;

/* MUST BE POWER OF 2 */
#define DEFAULT_QUEUE_SIZE 16

static can_cmb_t* q[DEFAULT_QUEUE_SIZE];
static int head;
static int tail;

int q_full;

void enqueue(can_cmb_t* q_item)
{
	q[tail] = q_item;
	int q_size = ((sizeof(q)/sizeof(q[0])) - 1);
	if(((tail + 1) & q_size) != head){
		tail = ((tail + 1) & q_size);
	}
}

can_cmb_t* dequeue(void)
{
	can_cmb_t* q_item = q[head];
	head = (head + 1) & ((sizeof(q)/sizeof(q[0])) - 1);
	return q_item;
}

can_cmb_t* queue_head_get(void)
{
	return q[head];
}

int queue_empty(void)
{
	return head == tail;
}

/* MUST BE POWER OF 2 */
#define DEFAULT_QUEUE_SIZE 16

static can_cmb_t q_overflow[DEFAULT_QUEUE_SIZE];
static int head_overflow;
static int tail_overflow;

int q_full;

void enqueue_overflow(can_cmb_t* q_item)
{
	memcpy(&q_overflow[tail_overflow], q_item, sizeof(can_cmb_t));
	int q_size = ((sizeof(q_overflow)/sizeof(q_overflow[0])) - 1);
	if(((tail_overflow + 1) & q_size) != head_overflow){
		tail_overflow = ((tail_overflow + 1) & q_size);
	}
}

can_cmb_t* dequeue_overflow(void)
{
	can_cmb_t* q_item = &q_overflow[head_overflow];
	head_overflow = (head_overflow + 1) & ((sizeof(q_overflow)/sizeof(q_overflow[0])) - 1);
	return q_item;
}

can_cmb_t* overflow_queue_head_get(void)
{
	return &q_overflow[head_overflow];
}

int overflow_queue_empty(void)
{
	return head_overflow == tail_overflow;
}


static uint32_t* can0_stack[DEFAULT_STACK_SIZE - 1];

static stack_t buffer_stack = {.array[0] = (void*)can0_stack};

static uint32_t* can1_stack[DEFAULT_STACK_SIZE - 1];

static stack_t buffer1_stack = {.array[0] = (void*)can1_stack};

volatile uint32_t* can_0_buffers[] = {&VOR_CAN0->CNSTAT_CMB0,  &VOR_CAN0->CNSTAT_CMB1,  &VOR_CAN0->CNSTAT_CMB2,
									&VOR_CAN0->CNSTAT_CMB3,  &VOR_CAN0->CNSTAT_CMB4,  &VOR_CAN0->CNSTAT_CMB5,
									&VOR_CAN0->CNSTAT_CMB6,  &VOR_CAN0->CNSTAT_CMB7,  &VOR_CAN0->CNSTAT_CMB8,
									&VOR_CAN0->CNSTAT_CMB9,  &VOR_CAN0->CNSTAT_CMB10, &VOR_CAN0->CNSTAT_CMB11,
									&VOR_CAN0->CNSTAT_CMB12, &VOR_CAN0->CNSTAT_CMB13/*, &VOR_CAN0->CNSTAT_CMB14*/ };


volatile uint32_t* can_1_buffers[] = {&VOR_CAN1->CNSTAT_CMB0,  &VOR_CAN1->CNSTAT_CMB1,  &VOR_CAN1->CNSTAT_CMB2,
								   	  &VOR_CAN1->CNSTAT_CMB3,  &VOR_CAN1->CNSTAT_CMB4,  &VOR_CAN1->CNSTAT_CMB5,
									  &VOR_CAN1->CNSTAT_CMB6,  &VOR_CAN1->CNSTAT_CMB7 , &VOR_CAN1->CNSTAT_CMB8,
									  &VOR_CAN1->CNSTAT_CMB9,  &VOR_CAN1->CNSTAT_CMB10, &VOR_CAN1->CNSTAT_CMB11,
									  &VOR_CAN1->CNSTAT_CMB12, &VOR_CAN1->CNSTAT_CMB13/*, &VOR_CAN1->CNSTAT_CMB14*/ };


typedef struct rx_buffer_info{
	can_cmb_t* rx_buf;
	uint32_t buf_num; 
} rx_buffer_info_t;

#define CAN_0_RX_NUM (sizeof(can_0_buffers)/sizeof(can_0_buffers[0]))
#define CAN_1_RX_NUM (sizeof(can_1_buffers)/sizeof(can_1_buffers[0]))

static rx_buffer_info_t can_0_rx_bufs[CAN_0_RX_NUM] = {0};
static rx_buffer_info_t can_1_rx_bufs[CAN_1_RX_NUM] = {0};

typedef struct can_if_ops{
	volatile uint32_t** can_bufs;
	VOR_CAN_Type*       can_ctrl;
	rx_buffer_info_t*   rx_buf;
	const unsigned int  rx_buf_length;
	const unsigned int  irq_num;
	stack_t*            stack;
	unsigned int        rx_buffer_bit_mask;
	volatile uint32_t   rx_data_buff_rdy;
	int                 if_num;
} can_if_ops_t;

static can_if_ops_t can_if_ops[VORAGO_CAN_INTERFACE_NUM] = {{can_0_buffers, VOR_CAN0, can_0_rx_bufs, CAN_0_RX_NUM, CAN0_IRQn, &buffer_stack, 0, 0, CAN_0_IF_NUM},
															{can_1_buffers, VOR_CAN1, can_1_rx_bufs, CAN_1_RX_NUM, CAN1_IRQn, &buffer1_stack, 0, 0, CAN_1_IF_NUM}};

#define CAN_IF_OPS_SIZE (sizeof(can_if_ops)/sizeof(can_if_ops[0]))


static volatile unsigned int error_irq;
static volatile unsigned int error_diag;
static volatile unsigned int error_count_rx;
static volatile unsigned int error_count_tx;
volatile int rx_count;

/** 
 * CAN ISR function 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return void 
 */
static inline void interrupt_func(int handle)		
{
	int xHigherPriorityTaskWoken = 0;

	VOR_CAN_Type* can_ctrl = can_if_ops[handle].can_ctrl;

	/* Find buffers that may have requested interrupts */
	volatile int interrupt_pending_0 = can_ctrl->CIPND;
	volatile int status_pending = can_ctrl->CSTPND;

	/* bit shift value of pending buffer to bit 0, and conver to bit position */
	volatile int ist = (status_pending & CAN_CSTPND_IST_Msk) >> CAN_CSTPND_IST_Pos;
	volatile int pnd_to_buf_num = (1 << (ist - 1));

	volatile can_cmb_t *can_cmb = 0;
	volatile int status = 0;

	int	irq = (status_pending & CAN_CSTPND_IRQ_Msk) >> CAN_CSTPND_IRQ_Pos;
	int	ns  = (status_pending & CAN_CSTPND_NS_Msk) >> CAN_CSTPND_NS_Pos;

	if(ist > 0){
		/* Find all the requesting buffer's status info */
		can_cmb = (can_cmb_t*)can_if_ops[handle].can_bufs[ist - 1];
		status  = can_cmb->CNSTAT & CAN_CNSTAT_CMB0_ST_Msk;
		if(interrupt_pending_0 & pnd_to_buf_num){
			/* TX interrupt */
			if((status) == en_can_cmb_cnstat_st_TX_NOT_ACTIVE){
				/* Reset buffer's status */
				can_cmb->CNSTAT = 0;

				/* Push the buffer that caused the interrupt and is finished sending back on the stack */
				stack_push(can_if_ops[handle].stack, (void*)can_cmb);

				if(ops.notify_tx_buff_avail_impl != 0){
					xHigherPriorityTaskWoken = ops.notify_tx_buff_avail_impl(&handle);
				}

				/* If we got this far, the buffer has already finished sending so dequeue it */
				if(!queue_empty()){
					dequeue();
				}

				/* If there's more to send, send them! */
				if(!queue_empty()){
					can_cmb = queue_head_get();
					can_driver_tx(can_cmb);
				}
			/* RX interrupt */
			} else if(status == en_can_cmb_cnstat_st_RX_FULL){
				/* Mark msg ready flag with the buffer number
				 * (should never be 0 if a buffer caused this interrupt */
				can_if_ops[handle].rx_data_buff_rdy = ist;
				
				/* Disable RX interrupts */
				can_ctrl->CIEN &= ~can_if_ops[handle].rx_buffer_bit_mask;
				__DSB();
				__ISB();

				if(ops.notify_task_impl != 0){
					xHigherPriorityTaskWoken = ops.notify_task_impl(&handle);
				}
			}

			/* Clear pending interrupt flag */
			can_ctrl->CICLR |= (pnd_to_buf_num);
		}
	} else if(irq && ist == 0){
		error_irq = irq;
		/* Clear pending interrupt */
		can_ctrl->CICLR |= (1 << CAN_CICLR_EICLR_Pos);

		/* Get error info */
		error_diag = can_ctrl->CEDIAG;
		error_count_tx = can_ctrl->CANEC & 0xFF;
		error_count_rx = (can_ctrl->CANEC & 0xFF00) >> 8;

		if(error_count_tx > 0 && (error_diag & CAN_CEDIAG_TXE_Msk)){
			/* If there was a tx error, a buffer may have gotten stuck so abort it */
			for(unsigned int i = 0; i < sizeof(can_0_buffers)/sizeof(can_0_buffers[0]); i++){
				volatile uint32_t cnstat = ((volatile can_cmb_t*)can_if_ops[handle].can_bufs[i])->CNSTAT;
				if(((cnstat & CAN_CNSTAT_CMB0_ST_Msk) == en_can_cmb_cnstat_st_TX_BUSY0) ||
				   ((cnstat & CAN_CNSTAT_CMB0_ST_Msk) == en_can_cmb_cnstat_st_TX_BUSY2) ){

					/* Unjam the buffer */
					volatile can_cmb_t* c = ((volatile can_cmb_t*)can_if_ops[handle].can_bufs[i]);
					c->CNSTAT = (uint32_t)en_can_cmb_cnstat_st_TX_NOT_ACTIVE;

					

					/* Push the buffer back on to the stack */
					stack_push(can_if_ops[handle].stack, (void*)c);


					/* Inform the task the buffer's ready and dequeue the msg */
					if(ops.notify_tx_buff_avail_impl != 0){
						xHigherPriorityTaskWoken = ops.notify_tx_buff_avail_impl((void*)&handle);
					}

					if(!queue_empty()){
						dequeue();
					}
					/* Record the error in the module error log when we get there */
				}
			}
		} else if(error_count_rx > 0) {
			/* Record the error in the module error log when we get there */
		}

		if(ns){
			/* Record the error in the module error log when we get there */
		}
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}	
																			
/* ISR for CAN 0 */																		
void CAN0_IRQHandler(void)
{
	interrupt_func(0);
}

/* ISR for CAN 1 */
void CAN1_IRQHandler(void)
{
	interrupt_func(1);
}

/**
 * Configure CAN buffers as receivers 
 *   
 * @param handle handle to the CAN interface 
 * @param id CAN ID to use for buffer acceptance filtering 
 * @param num_rxs Number of CAN buffers to configure this way
 *   
 * @return 0 on success !0 on failure 
 */
/* rx_buf_num is set by can_driver_init and used to indicate which buffer number to begin configuring receivers at */ 
static int rx_buf_num;
static int rx_config(const int handle, const uint32_t id, const uint32_t num_rxs)
{
	/* Hardware buffers are indexed at 1 */
	int err = 0;
	if(handle < 0 || handle >= (int)CAN_IF_OPS_SIZE){
		err= __LINE__;
	}else{
		for(uint32_t i = 0; i < num_rxs; i++){
			rx_buffer_info_t* rx_b = 0;

			/* Keep track of what buffers are configured as RX*/
			can_if_ops[handle].rx_buffer_bit_mask |= (1 << rx_buf_num);

			can_cmb_t* can_cmb = (can_cmb_t*)can_if_ops[handle].can_bufs[rx_buf_num];

			rx_b = can_if_ops[handle].rx_buf;

			can_cmb->CNSTAT = en_can_cmb_cnstat_st_RX_NOT_ACTIVE;
			can_cmb->ID1    = id << 5;
			can_cmb->CNSTAT = en_can_cmb_cnstat_st_RX_READY;

			int current_buffer = rx_buf_num - 1;
			rx_b[current_buffer].rx_buf  = can_cmb;
			rx_b[current_buffer].buf_num = rx_buf_num;
			rx_buf_num++;
		}
	}

	return err;
}

/**
 * Get data out of message buffer 
 *   
 * @param handle handle to the CAN interface 
 * @param rx_data_buff_rdy buffer number that's ready for processing 
 *   
 * @return void 
 */
static void can_driver_recv_message(const int handle, const uint32_t rx_data_buff_rdy)
{
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE && rx_data_buff_rdy > 0){    
		can_cmb_t* can_cmb = (can_cmb_t*)can_if_ops[handle].can_bufs[rx_data_buff_rdy - 1];

		message_t msg = {.id = can_cmb->ID1 >> 5,
							 .dlc = (can_cmb->CNSTAT & CAN_CNSTAT_CMB0_DLC_Msk) >> CAN_CNSTAT_CMB0_DLC_Pos};

		/* CAN data bytes reside in the lower 16 bits of the 4, 32 bit data words */
		*((uint16_t*)&msg.data[0]) = (uint16_t)can_cmb->DATA0;
		*((uint16_t*)&msg.data[2]) = (uint16_t)can_cmb->DATA1;
		*((uint16_t*)&msg.data[4]) = (uint16_t)can_cmb->DATA2;
		*((uint16_t*)&msg.data[6]) = (uint16_t)can_cmb->DATA3;

		/* Reset buffer as a receiver */
		can_cmb->CNSTAT = en_can_cmb_cnstat_st_RX_READY;

		/* Queue de message */
		if(ops.queue_message_impl != 0){
			ops.queue_message_impl(handle, &msg);
		}
	}
}


/** 
 * Find buffers that are ready for processing 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return buffer number that's ready for processing, or 0 if no buffers are ready  
 */
static uint32_t can_driver_message_ready(const int handle)
{
	static uint32_t i = 0;
	
	uint32_t buf_num = 0;
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE){    
		while(i < can_if_ops[handle].rx_buf_length && can_if_ops[handle].rx_buf[i].rx_buf != 0 && buf_num == 0){
			if(can_if_ops[handle].rx_buf[i].rx_buf->CNSTAT == en_can_cmb_cnstat_st_RX_FULL){
				can_if_ops[handle].can_ctrl->CICLR |= (1 << can_if_ops[handle].rx_buf[i].buf_num);
				buf_num = can_if_ops[handle].rx_buf[i].buf_num;
			} else {
				i++;
			}
		}

		if(i >= can_if_ops[handle].rx_buf_length || can_if_ops[handle].rx_buf[i].rx_buf == 0){
			i = 0;
		}
	}	
	
	return buf_num == 0 ? 0 : buf_num;
}



/** 
 * Process any RX'd messages that ready for processing 
 *   
 * @param handle handle to the CAN interface 
 * @param num_msgs_to_process Number of messages to process before yeilding the processor
 *   
 * @return 0 if successful or !0 on failure 
 */
int can_driver_process_rx(const int handle, const unsigned int num_msgs_to_process)
{
	int err = 0;
	if(handle < 0 || handle >= (int)CAN_IF_OPS_SIZE){
		err = __LINE__;
	} else {
		uint32_t more_message_available = can_if_ops[handle].rx_data_buff_rdy;
		unsigned int msgs_processed = 0;
		do {
			can_driver_recv_message(handle, more_message_available);
			more_message_available = can_driver_message_ready(handle);
		} while(more_message_available && msgs_processed++ < num_msgs_to_process);

		/* Re-enable buffer interrupts */
		can_if_ops[handle].can_ctrl->CIEN |= can_if_ops[handle].rx_buffer_bit_mask;
		__DSB();
		__ISB();
	}

	return err;
}


/** 
 * Get a TX buffer off the TX buffer stack 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return  pointer to buffer popped, or null if stack is empty
 */
can_cmb_t* can_buffer_get(const int handle)
{
	can_cmb_t* can_cmb = 0;
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE){    
		can_cmb = (can_cmb_t*)stack_pop(can_if_ops[handle].stack);
	}

	return can_cmb; 
}


/** 
 * Get the mask that indicates which buffers are configured as RX 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return 0 if no buffers are configured as RX, !0 indicating which buffers are configured as RX 
 */
uint32_t rx_buffer_mask_get(const int handle)
{
	uint32_t bit_mask = 0;
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE){    
		bit_mask = can_if_ops[handle].rx_buffer_bit_mask;
	}

	return bit_mask;
}


/** 
 * Get the RX ready status of an interface 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return 0 if no buffers are ready, !0 indicating which buffers are ready 
 */
uint32_t rx_data_buff_ready_get(const int handle)
{
	uint32_t data_buff_ready = 0;
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE){    
		data_buff_ready = can_if_ops[handle].rx_data_buff_rdy;
	}

	return data_buff_ready; 
}


/** 
 * Reset the status of a buffers that's been processed 
 *   
 * @param handle handle to the CAN interface 
 *   
 * @return void 
 */
void rx_data_buff_ready_reset(const int handle)
{
	if(handle >= 0 && handle < (int)CAN_IF_OPS_SIZE){    
		can_if_ops[handle].rx_data_buff_rdy = 0;
	}
}


/** 
 * Hardware level TX  
 *   
 * @param can_cmb hardware buffer pointer to configure for sending 
 *   
 * @return 0 on success, !0 on failure  
 */
int can_driver_tx(volatile can_cmb_t* can_cmb)
{
	int err = 0;
	if(can_cmb == 0){
		err = __LINE__;
	} else {
		can_cmb->CNSTAT &= ~CAN_CNSTAT_CMB0_ST_Msk;
		can_cmb->CNSTAT |= (uint32_t)en_can_cmb_cnstat_st_TX_NOT_ACTIVE;
		can_cmb->CNSTAT |= (uint32_t)en_can_cmb_cnstat_st_TX_ONCE;
        int i = 0;
        /* TODO find more sohpisticated timeout scheme than a for loop counter */
        while((can_cmb->CNSTAT & en_can_cmb_cnstat_st_TX_ONCE) == en_can_cmb_cnstat_st_TX_ONCE && (i++ < 10000));
	}
    
	return err;
}


/** 
 * Init the CAN driver 
 *    
 * @param can_init init structure for CAN driver
 *   
 * @return 0 on success, !0 on failure 
 */
int can_driver_init(can_init_t* can_init)
{
	static int if_count = 0;
	int err = 0;

    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_CAN0 | CLK_ENABLE_CAN1;
    VOR_SYSCONFIG->PERIPHERAL_RESET &= ~(SYSCONFIG_PERIPHERAL_RESET_CAN0_Msk | SYSCONFIG_PERIPHERAL_RESET_CAN1_Msk);
    __NOP();
    __NOP();
    VOR_SYSCONFIG->PERIPHERAL_RESET |= (SYSCONFIG_PERIPHERAL_RESET_CAN0_Msk | SYSCONFIG_PERIPHERAL_RESET_CAN1_Msk);

	if(if_count < 2){
		uint32_t buf_count = 0;
		for(int i = 0; i < can_init->rx_buffer_len; i++){
			buf_count += can_init->rx_buffers[i].rx_mob_count;
		}

		const uint32_t total_buffers = buf_count + can_init->tx_mob_count;

		if(total_buffers < VORAGO_TOTAL_BUFFERS){
			stack_init(can_if_ops[if_count].stack);

			for(int j = 0; j < (int)(sizeof(can_0_buffers)/sizeof(can_0_buffers[0])); j++){
				((can_cmb_t*)can_if_ops[if_count].can_bufs[j])->CNSTAT = en_can_cmb_cnstat_st_RX_NOT_ACTIVE;
				if(j < can_init->tx_mob_count){
					stack_push(can_if_ops[if_count].stack, (void*)can_if_ops[if_count].can_bufs[j]);
				}
			} 
			
			rx_buf_num = can_init->tx_mob_count;
			VOR_CAN_Type* vor_can = can_if_ops[if_count].can_ctrl;
			
			for(int i = 0; i < can_init->rx_buffer_len && !err; i++){
				if(can_init->rx_buffers[i].filter_type == CAN_FILTER_RANGE){
					
					/* 'Or' the two filter masked together and 'or' 0xF to the bottom 8 bits so the acceptance filter will pass
					 	 a range of values in the last byte (e.g. 0x280 - 0x285) while filtering on the second byte. 
						 This is buffer level filtering
					 */
					const unsigned int filter_bits =
							((can_init->rx_buffers[i].filter_high |
								can_init->rx_buffers[i].filter_low) & 0xFF) | 0xF;
					vor_can->GMSKB |= filter_bits << 5;

					/* Configure global filtering of to filter on first 3 bits of byte 2 */
					const unsigned int id_bits =
							((can_init->rx_buffers[i].filter_high |
								can_init->rx_buffers[i].filter_low) & 0x700);
					err = rx_config(if_count, id_bits, can_init->rx_buffers[i].rx_mob_count);

				} else if(can_init->rx_buffers[i].filter_type == CAN_FILTER_ID){
					
					/* If we're not filtering on a range, just set the buffer ID to accept */
					if(can_init->rx_buffers[i].filter_high == can_init->rx_buffers[i].filter_low){
						err = rx_config(if_count, can_init->rx_buffers[i].filter_high, can_init->rx_buffers[i].rx_mob_count);
					} else {
						err = -1;
					}
				} else {
					err = -1;
				}
			}

			if(!err){
                VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= (if_count) ? CLK_ENABLE_CAN0 : CLK_ENABLE_CAN1;

				uint32_t can_timing = 0;
				switch(can_init->baud){
					case CAN_BAUD_RATE_1000:
						can_timing = ((5-1) << CAN_CTIM_TSEG2_Pos) |
									 ((4-1) << CAN_CTIM_TSEG1_Pos) |
									 ((4-1) << CAN_CTIM_SJW_Pos)   |
									 ((4-1) << CAN_CTIM_PSC_Pos);

						vor_can->CTIM = can_timing;
						break;
				}

				vor_can->CICEN = ENABLE_ALL_INTERRUPTS; 
				vor_can->CGCR = 0x0
						| CAN_CGCR_BUFFLOCK_Msk
						| CAN_CGCR_TSTPEN_Msk
						| CAN_CGCR_DIAGEN_Msk
						| CAN_CGCR_CANEN_Msk;

				NVIC_SetPriority(can_if_ops[if_count].irq_num, IRQ_PRIORITY_SET(can_init->irq_priority));
				NVIC_EnableIRQ(can_if_ops[if_count].irq_num); // Enable IRQ
				vor_can->CIEN = ENABLE_ALL_INTERRUPTS;
				__DSB();
				__ISB();
			}
		}
	}
	return err == -1 ? err : can_if_ops[if_count++].if_num;
}
