#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "string.h"
#include "checksum.h"
#include "stdint.h"
#include "irq_router_driver_vorago.h"
#include "dma_driver_vorago.h"
#include "driver_serial_va41630_rtos.h"
#include "serial/uart_driver_vorago.h"
#include "cmsis/cmsis_gcc.h"
#include "osal/osal.h"


#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */

typedef struct serial_channel_init {
  uint32_t serial_handle;
  dma_channel_init_t *rx_dma_init;
  dma_channel_init_t *tx_dma_init;
  // See vorago datasheet for information on irq routing and dma triggering
  uint32_t rx_dma_channel_trigger;
  uint32_t tx_dma_channel_trigger;
  uint32_t rx_task_prio;
  uint32_t tx_task_prio;
} serial_channel_init_t;

typedef union {
  serial_frame_t serial_frame;
  uint8_t frame_bytes[sizeof(serial_frame_t)];
} serial_frame_data_t;
#pragma pack(pop)

#define FREERTOS_IRQ_PRIORITY(__X__) (5 + __X__)

#define FRAME_SIZE_MINUS_CRC ((sizeof(serial_frame_t) - sizeof((serial_frame_t*)0)->crc))

#define NUM_DMA_CHAIN_BUFFERS 8

#define SERIAL_TASK_STACK_SIZE   ( configMINIMAL_STACK_SIZE)

#define BUFFER_COUNT_RX 16
#define BUFFER_COUNT_TX 16

#define QUEUE_ITEM_SIZE_TX sizeof(message_t)
#define QUEUE_ITEM_SIZE_RX sizeof(message_t)
#define QUEUE_RX_STATIC_SIZE  QUEUE_ITEM_SIZE_RX * BUFFER_COUNT_RX
#define QUEUE_TX_STATIC_SIZE  QUEUE_ITEM_SIZE_TX * BUFFER_COUNT_TX

#define SERIAL_FRAME_SIZE sizeof(serial_frame_t)

#define SERIAL_SOF_BYTE 0xA8
#define SERIAL_SOF      SERIAL_SOF_BYTE << 8
#define SERIAL_SOF_MASK 0xF8

static uint8_t *serial_app_memory_get(const int handle, const dma_serial_channel_type_t chan_type);

typedef enum {
  DMA_CHANNEL_0,
  DMA_CHANNEL_1,
  DMA_CHANNEL_2,
  DMA_CHANNEL_3,
} dma_channel_num_t;

#define MAX_RETRIES 10

typedef struct {
  const unsigned int uart_handle;
  unsigned int rx_irq_trigger;
  unsigned int tx_irq_trigger;
  QueueHandle_t rx_q_handle;
  QueueHandle_t tx_q_handle;
  StaticQueue_t rx_q;
  StaticQueue_t tx_q;
  const unsigned int rx_q_item_size;
  const unsigned int tx_q_item_size;
  const unsigned int rx_q_buffer_count;
  const unsigned int tx_q_buffer_count;
  uint8_t rx_q_storage_area[QUEUE_RX_STATIC_SIZE];
  uint8_t tx_q_storage_area[QUEUE_RX_STATIC_SIZE];
  int rx_dma_handle;
  int tx_dma_handle;
  SemaphoreHandle_t tx_dma_sem_handle;
  StaticSemaphore_t tx_dma_sem_buffer;
  SemaphoreHandle_t rx_dma_sem_handle[NUM_DMA_CHAIN_BUFFERS];
  StaticSemaphore_t rx_dma_sem_buffer[NUM_DMA_CHAIN_BUFFERS];
  serial_frame_data_t rx_dma_chain[NUM_DMA_CHAIN_BUFFERS];
  serial_frame_data_t tx_dma_buffer;
  uint32_t rx_dma_sem_head;
  uint32_t rx_dma_sem_tail;
  StackType_t rx_task_stack[SERIAL_TASK_STACK_SIZE];
  StackType_t tx_task_stack[SERIAL_TASK_STACK_SIZE];
  StaticTask_t rx_task_buffer;
  StaticTask_t tx_task_buffer;
  TaskHandle_t rx_task_handle;
  TaskHandle_t tx_task_handle;
  const char *rx_task_name;
  const char *tx_task_name;
  bool if_initd;
} serial_ops_t;

static serial_ops_t serial_ops[UART_NUM] = {{.uart_handle = UART_IF_ID_0,
    .rx_irq_trigger = IRQ_ROUTER_TRIGGER_UART0_RX, .tx_irq_trigger = IRQ_ROUTER_TRIGGER_UART0_TX,
    .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
    .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
    .rx_task_name = "serial 0 rx task", .tx_task_name = "serial 0 tx task",
    .rx_dma_handle = -1, .tx_dma_handle = -1},

    {.uart_handle = UART_IF_ID_1,
        .rx_irq_trigger = IRQ_ROUTER_TRIGGER_UART1_RX, .tx_irq_trigger = IRQ_ROUTER_TRIGGER_UART1_TX,
        .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
        .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
        .rx_task_name = "serial 1 rx task", .tx_task_name = "serial 1 tx task",
        .rx_dma_handle = -1, .tx_dma_handle = -1},

    {.uart_handle = UART_IF_ID_2,
        .rx_irq_trigger = IRQ_ROUTER_TRIGGER_UART2_RX, .tx_irq_trigger = IRQ_ROUTER_TRIGGER_UART2_TX,
        .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
        .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
        .rx_task_name = "serial 2 rx task", .tx_task_name = "serial 2 tx task",
        .rx_dma_handle = -1, .tx_dma_handle = -1}};

#define SIZEOF_SERIAL_OPS (sizeof(serial_ops)/sizeof(serial_ops[0]))


static void DMA_Done_IRQFunc(int dma_channel_id)
{
    serial_ops_t *serial_op = 0;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    for(unsigned int i = 0; i < SIZEOF_SERIAL_OPS && serial_op == 0; i++) {
        if(serial_ops[i].rx_dma_handle == dma_channel_id) {
            serial_op = &serial_ops[i];
            int dma_handle = serial_op->rx_dma_handle;
            /* Reset channel settings and re-enable the DMA channel */
            dma_channel_reset(dma_handle);

            /* Give semaphore to indicate a message is ready for processing */
            uint32_t sem_head = serial_op->rx_dma_sem_head;
            xSemaphoreGiveFromISR(serial_op->rx_dma_sem_handle[sem_head], &xHigherPriorityTaskWoken);

            /* Update destination pointer to point to next buffer in DMA chain */
            serial_op->rx_dma_sem_tail = (serial_op->rx_dma_sem_tail + 1) & (NUM_DMA_CHAIN_BUFFERS - 1);
            uint32_t rx_tail = serial_op->rx_dma_sem_tail;
            uint32_t frame_end_index = sizeof(serial_op->rx_dma_chain[rx_tail].frame_bytes) - 1;
            dma_address_update(dma_handle, 0, &serial_op->rx_dma_chain[rx_tail].frame_bytes[frame_end_index]);

        }else if(serial_ops[i].tx_dma_handle == dma_channel_id) {
            serial_op = &serial_ops[i];
            int dma_handle = serial_op->tx_dma_handle;
            xSemaphoreGiveFromISR(serial_op->tx_dma_sem_handle, &xHigherPriorityTaskWoken);
            dma_channel_reset(dma_handle);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DMA_Done_0_IRQHandler(void)
{
    DMA_Done_IRQFunc(DMA_CHANNEL_0);
}

void DMA_Done_1_IRQHandler(void)
{
    DMA_Done_IRQFunc(DMA_CHANNEL_1);
}

void DMA_Done_2_IRQHandler(void)
{
    DMA_Done_IRQFunc(DMA_CHANNEL_2);
}

void DMA_Done_3_IRQHandler(void)
{
    DMA_Done_IRQFunc(DMA_CHANNEL_3);
}

/* 
 * Resynchronize frame in DMA chain 
 */
static void uart_frame_resync(const unsigned int handle, uint8_t *sfd)
{
    int i = 0;
    int sof_found = 0;
    const int frame_size = sizeof(serial_frame_t);
    /* Scan through all bytes in the buffer */
    for(; i < frame_size && !sof_found; i++) {
        /*  If we find another potential SOF byte ... */
        if((sfd[i] & SERIAL_SOF_MASK) == SERIAL_SOF_BYTE) {
            int sem_tail = serial_ops[handle].rx_dma_sem_tail;

            /* Copy the rest of the frame into the next open DMA buffer */
            memcpy(&serial_ops[handle].rx_dma_chain[sem_tail], &sfd[i], (frame_size - i));

            /* Set the DMA channel parameters to interrupt when the 
               remaining amount of bytes have been received */
            dma_transaction_length_set(serial_ops[handle].rx_dma_handle, i);
            uart_fifo_irq_trg_set(handle, i);
            sof_found = 1;
        }
    }
}

/*
 * Check if we received a valid frame
 */
static int uart_frame_validate(serial_frame_data_t *frame)
{
    int err = 0;

    uint16_t crc16 = 0;
    uint16_t sof_mask = frame->serial_frame.sof & SERIAL_SOF_MASK;

    if(sof_mask != SERIAL_SOF_BYTE) {
        err = __LINE__;
    }

    if(!err) {
        // @TODO #define magic number
        for(uint8_t i = 0; i < FRAME_SIZE_MINUS_CRC; i++) {
            uint8_t c = frame->frame_bytes[i];
            crc16 = update_crc_16(crc16, c);
        }

        if(crc16 != frame->serial_frame.crc) {
            err = __LINE__;
        }
    }

    return err;
}

/*
 * Send a serial message  
 */
static int serial_msg_send(int handle, message_t *msg)
{
    serial_frame_data_t *s = (serial_frame_data_t *)&serial_ops[handle].tx_dma_buffer.serial_frame;
    uint16_t crc = 0;

    //Set sof
    s->serial_frame.sof = __REV16((SERIAL_SOF | msg->id));
    s->serial_frame.dlc = msg->dlc;
    memcpy(s->serial_frame.data, msg->data, sizeof(msg->data));

    for(uint32_t i = 0; i < FRAME_SIZE_MINUS_CRC; i++) {
        uint8_t c = s->frame_bytes[i];
        crc = update_crc_16(crc, c);
    }
    s->serial_frame.crc = crc;

    /*
     * Make sure uart is done before DAM'ing more memory into its FIFO
     */
    while(uart_tx_status_get(handle)){
    }

    dma_sw_request(serial_ops[handle].tx_dma_handle);

    return 0;
}

/*
 * Receive a Serial Message  
 */
static void serial_rx_task(void *arg)
{
    serial_ops_t *serial_op = (serial_ops_t *)arg;
    int timeout = 0x0;
    while(1) {
        uint32_t rx_dma_sem_head = serial_op->rx_dma_sem_head;
        int status = xSemaphoreTake(serial_op->rx_dma_sem_handle[rx_dma_sem_head], portMAX_DELAY);

        /* Check if a message needs processing */
        if(status == pdTRUE) {
            int uart_handle = serial_op->uart_handle;

            // Check semaphore count for overflow conditions
            int count = uxSemaphoreGetCount(serial_op->rx_dma_sem_handle[rx_dma_sem_head]);
            if(count > 0) {
                // @TODO error handling
                //ERROR_SET(SERIAL_OVERFLOW);
            }

            int err = 0;
            serial_frame_data_t *serial_frame = &serial_op->rx_dma_chain[rx_dma_sem_head];
            int retries = 0;
            err = uart_frame_validate(serial_frame);
            if(err) {
                // if the frame couldn't be validated, read another frame
                uart_frame_resync(uart_handle, serial_frame->frame_bytes);
            }

            if(!err && retries < MAX_RETRIES) {
                message_t msg = {.id = __REV16(serial_frame->serial_frame.sof) & (0x7ff),
                    .dlc = serial_frame->serial_frame.dlc};

                memcpy(msg.data, &serial_frame->serial_frame.data, sizeof(msg.data));
                xQueueSend(serial_op->rx_q_handle, &msg, timeout);
            }

            serial_op->rx_dma_sem_head = (serial_op->rx_dma_sem_head + 1) & (NUM_DMA_CHAIN_BUFFERS - 1);
        }
    }
}

/*
 * Transmit a Serial Message  
 */
static void serial_tx_task(void *arg)
{
    serial_ops_t *serial_op = (serial_ops_t *)arg;
    while(1) {

        message_t msg = {0};

        int err = 0;
        // Try to take the semaphore to determine if the DMA engine and UART are ready for transmitting
        int status = xSemaphoreTake(serial_op->tx_dma_sem_handle,  portMAX_DELAY);
        if(status == pdTRUE) {
            // Don't dequeue until we know we can send the message
            status = xQueueReceive(serial_op->tx_q_handle, &msg, portMAX_DELAY);
            if(status == pdTRUE) {

                int retries = 0;

                do {
                    err = serial_msg_send(serial_op->uart_handle, &msg);
                } while(err && retries++ < MAX_RETRIES);

                // If we're still unsuccessful despite our best attempts...
                if(err && retries == MAX_RETRIES) {
                    // @TODO error handling 
                    //ERROR_SET(UART_TX_FAILED); // Flush buffer and reset peripheral or whatever is needed to get us talking again
                }
            }
        }
    }
}

/*
 * Init everything necessary for Serial Comms 
 */
static int serial_channel_create(const serial_channel_init_t *serial_init)
{
    static int initd = 0;
    int err = 0;

    // Only initialize these perihperals the first time we make a channel
    if(!initd) {
        dma_init();
        irq_router_init();
        initd = 1;
    }

    if(serial_init == 0) {
        err = -1;
    }else {
        uint32_t serial_id = serial_init->serial_handle;
        serial_ops_t *s = &serial_ops[serial_id];
        int rx_dma_handle = -1;
        int tx_dma_handle = -1;
        if(serial_init->rx_dma_init != 0) {
            rx_dma_handle = s->rx_dma_handle = dma_channel_create(serial_init->rx_dma_init, DMA_SERIAL_RX_CHANNEL);
        }

        if(serial_init->tx_dma_init != 0) {
            tx_dma_handle = s->tx_dma_handle = dma_channel_create(serial_init->tx_dma_init, DMA_SERIAL_TX_CHANNEL);
        }

        /* If DMA hasn'e been init'd properly, don't do anything else */
        if(rx_dma_handle == -1 && tx_dma_handle == -1) {
            err = -1;
        }else {
            /*  Set IRQ routing */
            if(rx_dma_handle != -1) {
                irq_router_dma_channel_trigger_set(rx_dma_handle, serial_init->rx_dma_channel_trigger);
            }

            if(tx_dma_handle != -1) {
                irq_router_dma_channel_trigger_set(tx_dma_handle, serial_init->tx_dma_channel_trigger);
            }

            for(int i = 0; i < NUM_DMA_CHAIN_BUFFERS; i++) {
                OSAL_SEM_CreateStatic(&s->rx_dma_sem_handle[i],
                                      OSAL_SEM_TYPE_COUNTING,
                                      1,
                                      0,
                                      &s->rx_dma_sem_buffer[i],
                                      "dma");
            }
            OSAL_QUEUE_CreateStatic(&s->rx_q_handle,
                                     s->rx_q_buffer_count,
                                     s->rx_q_item_size,
                                     s->rx_q_storage_area,
                                     &s->rx_q,
                                     "ser_rx");

            s->rx_task_handle = xTaskCreateStatic(serial_rx_task,
                                                  s->rx_task_name,
                                                  sizeof(s->rx_task_stack) / sizeof(s->rx_task_stack[0]),
                                                  (void *)&serial_ops[serial_id],
                                                  serial_init->rx_task_prio,
                                                  s->rx_task_stack,
                                                  &s->rx_task_buffer);

            OSAL_SEM_CreateStatic(&s->tx_dma_sem_handle, OSAL_SEM_TYPE_COUNTING, 1, 1, &s->tx_dma_sem_buffer, "tx_dma");
            OSAL_QUEUE_CreateStatic(&s->tx_q_handle, s->tx_q_buffer_count, s->tx_q_item_size,
                                    s->tx_q_storage_area, &s->tx_q, "tx_q");
            s->tx_task_handle = xTaskCreateStatic(serial_tx_task,
                                                  s->tx_task_name,
                                                  sizeof(s->tx_task_stack) / sizeof(s->tx_task_stack[0]),
                                                  (void *)&serial_ops[serial_id],
                                                  serial_init->tx_task_prio,
                                                  s->tx_task_stack,
                                                  &s->tx_task_buffer);
        }
    }
    return err;
}

int serial_channel_init(const uart_init_t *ui)
{
    dma_channel_init_t rx_dma_channel = {.irq_priority = FREERTOS_IRQ_PRIORITY(0),
        .dest_addr = serial_app_memory_get(ui->uart_if_id, DMA_SERIAL_RX_CHANNEL),
        .dest_increment = DMA_INCREMENT_BYTE,
        .src_addr  = uart_data_ptr_get(ui->uart_if_id),
        .src_increment = DMA_INCREMENT_NO_INCREMENT,
        .transaction_len = SERIAL_FRAME_SIZE,
        .cycle_control = DMA_CYCLE_CONTROL_PING_PONG};

    dma_channel_init_t tx_dma_channel = {.irq_priority = FREERTOS_IRQ_PRIORITY(0),
        .dest_addr = uart_data_ptr_get(ui->uart_if_id),
        .dest_increment = DMA_INCREMENT_NO_INCREMENT,
        .src_addr  = serial_app_memory_get(ui->uart_if_id, DMA_SERIAL_TX_CHANNEL),
        .src_increment = DMA_INCREMENT_BYTE,
        .transaction_len = SERIAL_FRAME_SIZE,
        .cycle_control = DMA_CYCLE_CONTROL_BASIC};

    serial_channel_init_t sci = {.serial_handle = ui->uart_if_id,
        .rx_dma_init = &rx_dma_channel,
        .tx_dma_init = &tx_dma_channel,
        .rx_dma_channel_trigger = serial_ops[ui->uart_if_id].rx_irq_trigger,
        .tx_dma_channel_trigger = serial_ops[ui->uart_if_id].tx_irq_trigger,
        .rx_task_prio = ui->rx_task_prio,
        .tx_task_prio = ui->tx_task_prio
    };

    return serial_channel_create(&sci);
}

/*
 * Return pointer to app memory to use for DMA source or destination
 */
uint8_t *serial_app_memory_get(const int handle, const dma_serial_channel_type_t chan_type)
{
    uint8_t *app_memory_ptr = 0;
    if(handle >= 0 && handle < (int)SIZEOF_SERIAL_OPS) {
        if(chan_type == DMA_SERIAL_RX_CHANNEL) {
            uint32_t frame_end_index = sizeof(serial_ops[handle].rx_dma_chain[0].frame_bytes) - 1;
            int sem_tail = serial_ops[handle].rx_dma_sem_tail;
            app_memory_ptr = &serial_ops[handle].rx_dma_chain[sem_tail].frame_bytes[frame_end_index];
        }else {
            uint32_t frame_end_index = sizeof(serial_ops[handle].tx_dma_buffer.frame_bytes) - 1;
            app_memory_ptr = &serial_ops[handle].tx_dma_buffer.frame_bytes[frame_end_index];
        }
    }
    return app_memory_ptr;
}

/**
 * write out uart peripheral.
 * @param handle
 * @param msg
 * @param timeout
 * @return
 */
int uart_write(int handle, message_t *msg, int timeout)
{
    int err = 0;

    if(handle >= 0 && handle < (int)SIZEOF_SERIAL_OPS && msg != NULL) {
        int status = xQueueSend(serial_ops[handle].tx_q_handle, msg, timeout);
        if(status != pdTRUE) {
            err = __LINE__;
        }
    } else {
        err = __LINE__;
    }

    return err;
}

/**
 * Read uart from peripheral.
 * @param handle
 * @param msg
 * @param timeout
 * @return
 */
int uart_read(const int handle, message_t *msg, int timeout)
{
    int err = 0;
    if(handle >= 0 && handle < (int)SIZEOF_SERIAL_OPS && msg != NULL) {
        int status = xQueueReceive(serial_ops[handle].rx_q_handle, msg, timeout);
        if(status != pdTRUE) {
            err = __LINE__;
        }
    }else {
        err = __LINE__;
    }
    return err;
}
