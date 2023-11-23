#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "string.h"
#include "checksum.h"
#include "stdint.h"

#include "driver_serial_va41630_rtos.h"
#include "serial/uart_driver_vorago.h"
#include "cmsis/cmsis_gcc.h"
#include "osal/osal.h"

#include <unistd.h>
#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */

typedef struct serial_channel_init {
    uint32_t serial_handle;
    //dma_channel_init_t *rx_dma_init;
    //dma_channel_init_t *tx_dma_init;
    // See vorago datasheet for information on irq routing and dma triggering
    uint32_t rx_dma_channel_trigger;
    uint32_t tx_dma_channel_trigger;
    uint32_t rx_task_prio;
    uint32_t tx_task_prio;
} serial_channel_init_t;

typedef struct dma_serial_channel_type {
    uint32_t nothing;
} dma_serial_channel_type_t;

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


typedef enum {
  DMA_CHANNEL_0,
  DMA_CHANNEL_1,
  DMA_CHANNEL_2,
  DMA_CHANNEL_3,
} dma_channel_num_t;

#define MAX_RETRIES 10

typedef struct {
  unsigned int uart_handle;
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
    .rx_irq_trigger = 0, .tx_irq_trigger = 0,
    .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
    .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
    .rx_task_name = "serial 0 rx task", .tx_task_name = "serial 0 tx task",
    .rx_dma_handle = -1, .tx_dma_handle = -1},

    {.uart_handle = UART_IF_ID_1,
        .rx_irq_trigger = 0, .tx_irq_trigger = 0,
        .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
        .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
        .rx_task_name = "serial 1 rx task", .tx_task_name = "serial 1 tx task",
        .rx_dma_handle = -1, .tx_dma_handle = -1},

    {.uart_handle = UART_IF_ID_2,
        .rx_irq_trigger = 0, .tx_irq_trigger = 0,
        .rx_q_item_size = QUEUE_ITEM_SIZE_RX, .rx_q_buffer_count = BUFFER_COUNT_RX,
        .tx_q_item_size = QUEUE_ITEM_SIZE_TX, .tx_q_buffer_count = BUFFER_COUNT_TX,
        .rx_task_name = "serial 2 rx task", .tx_task_name = "serial 2 tx task",
        .rx_dma_handle = -1, .tx_dma_handle = -1}};

#define SIZEOF_SERIAL_OPS (sizeof(serial_ops)/sizeof(serial_ops[0]))


/*
 * Resynchronize frame in DMA chain
 */
static void uart_frame_resync(const unsigned int handle, uint8_t *sfd)
{

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
    int status = write (handle, msg, sizeof(message_t));           // send 7 character greeting
    return status;
}

/*
 * Receive a Serial Message  
 */
#include <stdio.h>
static void serial_rx_task(void *arg)
{
    serial_ops_t *serial_op = (serial_ops_t *)arg;
    int timeout = 0x0;
    while(1) {
        message_t msg = {0};

        int status = read(serial_op->uart_handle, &msg, sizeof(message_t));
        if(status == sizeof(message_t)) {
            xQueueSend(serial_op->rx_q_handle, &msg, timeout);
        }
        vTaskDelay(1);
    }
}

/*
 * Transmit a Serial Message  
 */
#include <stdio.h>
static void serial_tx_task(void *arg)
{
    serial_ops_t *serial_op = (serial_ops_t *)arg;
    while(1) {
        message_t msg = {0};

        // Don't dequeue until we know we can send the message
        xQueueReceive(serial_op->tx_q_handle, &msg, portMAX_DELAY);
        printf("tx task\n");

        serial_msg_send(serial_op->uart_handle, &msg);
    }
}

/*
 * Init everything necessary for Serial Comms 
 */
static int serial_channel_create(const serial_channel_init_t *serial_init)
{
    static int initd = 0;
    int err = 0;

    if(serial_init == 0) {
        err = -1;
    }else {

        uint32_t serial_id = 0;

        serial_ops[serial_id].uart_handle = serial_init->serial_handle;

        serial_ops_t *s = &serial_ops[serial_id];

        OSAL_QUEUE_CreateStatic(&s->rx_q_handle,
                                 s->rx_q_buffer_count,
                                 s->rx_q_item_size,
                                 s->rx_q_storage_area,
                                 &s->rx_q,
                                 "ser_rx");

        OSAL_QUEUE_CreateStatic(&s->tx_q_handle,
                                s->tx_q_buffer_count,
                                s->tx_q_item_size,
                                s->tx_q_storage_area,
                                &s->tx_q,
                                "tx_q");

        s->rx_task_handle = xTaskCreateStatic(serial_rx_task,
                                              s->rx_task_name,
                                              sizeof(s->rx_task_stack) / sizeof(s->rx_task_stack[0]),
                                              (void *)&serial_ops[serial_id],
                                              serial_init->rx_task_prio,
                                              s->rx_task_stack,
                                              &s->rx_task_buffer);



        s->tx_task_handle = xTaskCreateStatic(serial_tx_task,
                                              s->tx_task_name,
                                              sizeof(s->tx_task_stack) / sizeof(s->tx_task_stack[0]),
                                              (void *)&serial_ops[serial_id],
                                              serial_init->tx_task_prio,
                                              s->tx_task_stack,
                                              &s->tx_task_buffer);
    }
    return err;
}

int serial_channel_init(const uart_init_t *ui)
{
    serial_channel_init_t sci = {.serial_handle = ui->uart_if_id,
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
    } else {
        err = __LINE__;
    }
    return err;
}
