#include "driver_can_va41630_bm.h"
#include "device.h"
#include "can/driver_can_va41630_cmn.h"

#define DEFAULT_QUEUE_SIZE 16
#define NUM_MSGS_TO_PROCESS 10

static can_cmb_t q_1[DEFAULT_QUEUE_SIZE];
static int head_q_1;
static int tail_q_1;

void enqueue_q_1(can_cmb_t* q_item)
{
    memcpy(&q_1[tail_q_1], q_item, sizeof(can_cmb_t));
    int q_size = ((sizeof(q_1)/sizeof(q_1[0])) - 1);
    if(((tail_q_1 + 1) & q_size) != head_q_1){
        tail_q_1 = ((tail_q_1 + 1) & q_size);
    }
}

can_cmb_t* dequeue_q_1(void)
{
    can_cmb_t* q_item = &q_1[head_q_1];
    head_q_1 = (head_q_1 + 1) & ((sizeof(q_1)/sizeof(q_1[0])) - 1);
    return q_item;
}

can_cmb_t* queue_1_head_get(void)
{
    return &q_1[head_q_1];
}

int queue_1_empty(void)
{
    return head_q_1 == tail_q_1;
}


static can_cmb_t q_2[DEFAULT_QUEUE_SIZE];
static int head_q_2;
static int tail_q_2;

void enqueue_q_2(can_cmb_t* q_item)
{
    memcpy(&q_2[tail_q_2], q_item, sizeof(can_cmb_t));
    int q_size = ((sizeof(q_2)/sizeof(q_2[0])) - 1);
    if(((tail_q_2 + 1) & q_size) != head_q_2){
        tail_q_2 = ((tail_q_2 + 1) & q_size);
    }
}

can_cmb_t* dequeue_q_2(void)
{
    can_cmb_t* q_item = &q_2[head_q_2];
    head_q_2 = (head_q_2 + 1) & ((sizeof(q_2)/sizeof(q_2[0])) - 1);
    return q_item;
}

can_cmb_t* queue_2_head_get(void)
{
    return &q_2[head_q_2];
}

int queue_2_empty(void)
{
    return head_q_2 == tail_q_2;
}


int can_notif_tx_buff_avail_BM(void* arg)
{
    return true;
}


typedef can_cmb_t* (*DEQUEUE)(void);
typedef can_cmb_t* (*QUEUE_HEAD_GET)(void);
typedef void (*ENQUEUE)(can_cmb_t*);
typedef int (*QUEUE_EMPTY)(void);

typedef struct can_bm_queue_ops{
  DEQUEUE dequeue_impl;
  QUEUE_HEAD_GET queue_head_get_impl;
  ENQUEUE enqueue_impl;
  QUEUE_EMPTY queue_empty_impl;
} can_bm_queue_ops_t;

typedef struct can_bm_ops {
  can_bm_queue_ops_t can_bm_queue_ops;
} can_bm_ops_t;

static can_bm_ops_t can_bm_ops[] = {{.can_bm_queue_ops.dequeue_impl = dequeue_q_1,
    .can_bm_queue_ops.enqueue_impl = enqueue_q_1,
    .can_bm_queue_ops.queue_empty_impl = queue_1_empty,
    .can_bm_queue_ops.queue_head_get_impl = queue_1_head_get},
    {.can_bm_queue_ops.dequeue_impl = dequeue_q_2,
        .can_bm_queue_ops.enqueue_impl = enqueue_q_2,
        .can_bm_queue_ops.queue_empty_impl = queue_2_empty,
        .can_bm_queue_ops.queue_head_get_impl = queue_2_head_get}};




/**
 * Translate an application level message into a hardware level message
 *
 * @param can_cmb hardware level message pointer
 * @param msg application level pointer to tranlate into hardware message pointer
 *
 * @return 0 on success or !0 on err
 */
static int can_message_create(can_cmb_t* can_cmb, message_t* msg)
{
    int err = 0;
    if(msg == 0){
        err = __LINE__;
    } else {
        can_cmb->CNSTAT |= (msg->dlc << CAN_CNSTAT_CMB0_DLC_Pos);
        can_cmb->ID1 = msg->id << 5;
        can_cmb->DATA0 = (msg->data[1] | (msg->data[0] << 8));
        can_cmb->DATA1 = (msg->data[3] | (msg->data[2] << 8));
        can_cmb->DATA2 = (msg->data[5] | (msg->data[4] << 8));
        can_cmb->DATA3 = (msg->data[7] | (msg->data[6] << 8));
    }
    return err;
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
    const unsigned int rx_buffer_bit_mask = rx_buffer_mask_get(handle);
    VOR_CAN0->CIEN &= rx_buffer_bit_mask;
    VOR_CAN1->CIEN &= rx_buffer_bit_mask;
    can_cmb_t* can_cmb = can_buffer_get(handle);

    if(can_cmb != 0){
        can_message_create(can_cmb, msg);
        if(queue_empty()){
            can_driver_tx(can_cmb);
        }
        enqueue(can_cmb);
    } else {
        can_cmb_t can_cmb_overflow = {0};
        can_message_create(&can_cmb_overflow, msg);
        enqueue_overflow(&can_cmb_overflow);
    }
    VOR_CAN0->CIEN |= ~rx_buffer_bit_mask;
    VOR_CAN1->CIEN |= ~rx_buffer_bit_mask;
    return 0;
}


/* Empty function */
static int can_init_bm(void)
{
    int err = 0;
    /* No extra init needed */
    return err;
}


/* Empty Function */
int can_notify_rx_task_BM(void)
{
    return true;
}


/**
 * Init the CAN driver
 *
 * @param can_init structure of init params
 *
 * @return 0 on success, -1 on failure
 */
int can_init(can_init_t *can_init)
{
    int handle = -1;
    int err = 0;
    err = can_driver_init(can_init);
    if(err != -1){
        handle = err;
        err = can_init_bm();
    }
    return err == -1 ? err : handle;
}


/**
 * Enqueue a received message for processing
 *
 * @param handle handle of interface
 * @param msg pointer to message to enqueue
 *
 * @return 0 on success, !0 on failure
 */
int can_queue_rx_message_BM(int handle, message_t* msg)
{
    int err = 0;
    if(msg == 0){
        err = __LINE__;
    } else {
        can_cmb_t can_cmb = {0};
        can_cmb.ID1 = msg->id << 5;
        can_cmb.DATA0 = (msg->data[1] | (msg->data[0] << 8));
        can_cmb.DATA1 = (msg->data[3] | (msg->data[2] << 8));
        can_cmb.DATA2 = (msg->data[5] | (msg->data[4] << 8));
        can_cmb.DATA3 = (msg->data[7] | (msg->data[6] << 8));
        can_bm_ops[handle].can_bm_queue_ops.enqueue_impl(&can_cmb);
    }

    return err;
}


/**
 * Use in application loop to poll for CAN messages that need processing
 *
 * @param handle handle of interface
 *
 * @return void
 */
void can_rx_service(const int handle)
{
    if(rx_data_buff_ready_get(handle)){
        can_driver_process_rx(handle, NUM_MSGS_TO_PROCESS);
        rx_data_buff_ready_reset(handle);
    }
}


/**
 * Receive a CAN message
 *
 * @param handle handle of interface
 * @param msg message pointer to receive in to
 * @param timeout timeout to wait for queue operations
 *
 * @return 0 on success !0 on failure
 */
uint32_t can_rcv(int handle, message_t* msg, int timeout)
{
    int err = 0;
    can_cmb_t* can_cmb = can_bm_ops[handle].can_bm_queue_ops.dequeue_impl();
    if(msg != 0){
        msg->id = can_cmb->ID1 >> 5;
        msg->dlc = (can_cmb->CNSTAT & CAN_CNSTAT_CMB0_DLC_Msk) >> CAN_CNSTAT_CMB0_DLC_Pos;

        *((uint16_t*)&msg->data[0]) = (uint16_t)(can_cmb->DATA0);
        *((uint16_t*)&msg->data[2]) = (uint16_t)(can_cmb->DATA1);
        *((uint16_t*)&msg->data[4]) = (uint16_t)(can_cmb->DATA2);
        *((uint16_t*)&msg->data[6]) = (uint16_t)(can_cmb->DATA3);
    } else {
        err = __LINE__;
    }

    return err;
}


/**
 * Determine if there's more message ready for processing
 *
 * @param handle handle to CAN interface
 *
 * @return 0 if queue is empty, !0 otherwise
 */
uint32_t can_rx_is_empty(int handle)
{
    return can_bm_ops[handle].can_bm_queue_ops.queue_empty_impl();
}
