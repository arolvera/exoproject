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
/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "definitions.h"                // SYS function prototypes

#include "serial_task.h"                // Header file for this module

#include "task.h"                       // RTOS Task Defines
#include "semphr.h"                     // RTOS Semaphore Defines

#include "include/serial_p.h"           // Serial Module Private Defines
#include "mcan/mcan_task.h"
#include "checksum.h"                   // CRC checker
#include "error/error_handler.h"
#include "utilities/trace/trace.h"        // Error handling
#include "thruster_control.h"             // for MODULE_NUM_SERIAL

#define MODULE_NUM MODULE_NUM_SERIAL
#define SUBMODULE_NUM COMMS_SERIAL_SUBMODULE

static void* serial_error_detail_strategy(void* arg);


#define DETAIL_LOG_LENGTH LOG_LENGTH_16


static serial_error_detail_t error_detail[DETAIL_LOG_LENGTH];



#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */

#define BUFFER_COUNT                    5
#define BUFFER_COUNT_TX                 BUFFER_COUNT
#define BUFFER_COUNT_CO                 BUFFER_COUNT

/* Wait ticks is how long it will sleep on a semaphore or queue waiting for 
 * and RX/TX event.   ERROR reset ticks is how many times WAIT TICK expires
 * and the COM interface is in an error state before it will automatically
 * reset.
 */
#define RX_TX_WAIT_TICKS             1000
#define RX_TX_ERROR_RESET_TICKS         5
static const TickType_t xRxTxTicksToWait = pdMS_TO_TICKS(RX_TX_WAIT_TICKS);

/* CAN compatible message for rcv queue */
typedef struct can_msg {
    uint32_t identifier;
    uint8_t dlc;
    uint8_t data[8];
} can_msg_t;

static TaskHandle_t serial_task_rx_handle;
static TaskHandle_t serial_task_tx_handle;

#define SERIAL_TASK_STACK_SIZE   ( configMINIMAL_STACK_SIZE * 3 )
#define SERIAL_TASK_RX_PRIORITY     ( tskIDLE_PRIORITY + 4)
#define SERIAL_TASK_TX_PRIORITY     ( tskIDLE_PRIORITY + 1)
/* TX Queue Item size */
#define QUEUE_ITEM_SIZE_TX       (sizeof(Serial_Frame_t))
/* Can Open Queue Item size */
#define QUEUE_ITEM_SIZE_CO       (sizeof(can_msg_t))

static QueueHandle_t xTxQueue;
static QueueHandle_t xCoQueue;

static QueueHandle_t *pxSDOQueue = NULL;

static StaticQueue_t xTxStaticQueue;
static StaticQueue_t xCoStaticQueue;

#define SERIAL_QUEUE_STATIC_SIZE (BUFFER_COUNT_TX * QUEUE_ITEM_SIZE_TX)
static uint8_t ucTxQueueStorageArea[ SERIAL_QUEUE_STATIC_SIZE * 2];
static uint8_t ucCoQueueStorageArea[ QUEUE_ITEM_SIZE_CO * BUFFER_COUNT_CO];

/* Buffer for the task being to use as its stack. */
static StackType_t rxStack[ SERIAL_TASK_STACK_SIZE ];
static StackType_t txStack[ SERIAL_TASK_STACK_SIZE ];

/* Structure that will hold the TCB of the task being created. */
static StaticTask_t rxTaskBuffer;
static StaticTask_t txTaskBuffer;

static StaticSemaphore_t txSemaphoreBuffer;

static SemaphoreHandle_t txDoneMsgSemaphore = NULL;

#define SERIAL_SOF          (0xA8) /* SERIAL Start of Frame upper 5 bits = 10101b */
#define SERIAL_SOF_MASK     (0xF8) /* Lower three bits are used as part of COBID  */

#define RX_DMA_BUFFERS      3
#define COMM_RESET_STATE    0   // Active LOW switch

static Serial_Frame_t rx_frame_buffers[RX_DMA_BUFFERS] __attribute__((aligned (32)))__attribute__((space(data), section (".ram_nocache")));
static Serial_Frame_t tx_frame_buffer __attribute__((aligned (32)))__attribute__((space(data), section (".ram_nocache")));
static XDMAC_DESCRIPTOR_VIEW_1 pRxLinkedListDesc[RX_DMA_BUFFERS];
/* Each DMA Buffer has a parallel counting semaphore to track RX events
 * and overruns.  Any count above 1 indicates an overrun */
// 10 outstanding per buffer is MORE than enough
#define COUNT_SEM_RX_MAX 10
static SemaphoreHandle_t rx_semaphores[RX_DMA_BUFFERS];
static StaticSemaphore_t rx_semaphores_buffers[RX_DMA_BUFFERS];
    
/* The buffer the DMA engine is currently using */
static volatile int dma_head = 0;
/* The buffer the RX task is using */
static volatile int dma_tail = 0;

/* Serial Frame resync status - Used when we have a COM error to track the
    status of the resync function */
static int frame_resynced = 0;

__attribute__((__aligned__(4))) static XDMAC_DESCRIPTOR_CONTROL rxFirstDescriptorControl =
{
    .fetchEnable = 1,
    .sourceUpdate = 1,
    .destinationUpdate = 1,
    .view = 1
};

/******************** Serial UART/USARTETUP ***********************************/
/* Older boards use the USART - New USART to take advantage of the RS-485 mode*/
#define UART0_DMA_CHANNEL_RX  XDMAC_CHANNEL_0
#define UART0_DMA_CHANNEL_TX  XDMAC_CHANNEL_1
#define USART1_DMA_CHANNEL_RX XDMAC_CHANNEL_2
#define USART1_DMA_CHANNEL_TX XDMAC_CHANNEL_3
#define USART2_DMA_CHANNEL_RX XDMAC_CHANNEL_4
#define USART2_DMA_CHANNEL_TX XDMAC_CHANNEL_5

#define UART0_IER_ADDRESS_RX     ((void *) &UART0_REGS->UART_IER)
#define UART0_HOLDNG_ADDRESS_RX  ((void *) &UART0_REGS->UART_RHR)
#define UART0_HOLDNG_ADDRESS_TX  ((void *) &UART0_REGS->UART_THR)
#define USART1_IER_ADDRESS_RX    ((void *) &USART1_REGS->US_IER)
#define USART1_HOLDNG_ADDRESS_RX ((void *) &USART1_REGS->US_RHR)
#define USART1_HOLDNG_ADDRESS_TX ((void *) &USART1_REGS->US_THR)
#define USART2_IER_ADDRESS_RX    ((void *) &USART2_REGS->US_IER)
#define USART2_HOLDNG_ADDRESS_RX ((void *) &USART2_REGS->US_RHR)
#define USART2_HOLDNG_ADDRESS_TX ((void *) &USART2_REGS->US_THR)


usart_registers_t *usart_regs = NULL;

/* Default to the old board */
static int dma_channel_rx    = UART0_DMA_CHANNEL_RX;
static int dma_channel_tx    = UART0_DMA_CHANNEL_TX;
static void *holding_addr_rx = UART0_HOLDNG_ADDRESS_RX;
static void *holding_addr_tx = UART0_HOLDNG_ADDRESS_TX;
static void *ie_addr_rx      = USART1_IER_ADDRESS_RX;

/* UART needs GPIO pin for RTS - USART is RS-485 Mode */
static inline void serial_uart_tx(void);
static inline void serial_usart_rs485_tx(void);
static inline void serial_usart_rs422_tx(void);
typedef void (*SERIAL_TX_FUNC) (void);
static SERIAL_TX_FUNC serial_tx = serial_uart_tx;

static void serial_dev_setup(serial_device_t *dev)
{
    switch(*dev) {
        case SERIAL_USART1 :
            USART1_REGS->US_MR |= US_MR_USART_MODE(US_MR_USART_MODE_RS485_Val);
            dma_channel_rx  = USART1_DMA_CHANNEL_RX;
            dma_channel_tx  = USART1_DMA_CHANNEL_TX;
            holding_addr_rx = USART1_HOLDNG_ADDRESS_RX;
            holding_addr_tx = USART1_HOLDNG_ADDRESS_TX;
            ie_addr_rx      = USART1_IER_ADDRESS_RX;
            serial_tx       = serial_usart_rs485_tx;
            usart_regs      = USART1_REGS;
            break;
        case SERIAL_USART2 :
            dma_channel_rx  = USART2_DMA_CHANNEL_RX;
            dma_channel_tx  = USART2_DMA_CHANNEL_TX;
            holding_addr_rx = USART2_HOLDNG_ADDRESS_RX;
            holding_addr_tx = USART2_HOLDNG_ADDRESS_TX;
            ie_addr_rx      = USART2_IER_ADDRESS_RX;
            serial_tx       = serial_usart_rs422_tx;
            usart_regs      = USART2_REGS;
            break;
        case SERIAL_UART0 :
        default:
            dma_channel_rx  = UART0_DMA_CHANNEL_RX;
            dma_channel_tx  = UART0_DMA_CHANNEL_TX;
            holding_addr_rx = UART0_HOLDNG_ADDRESS_RX;
            holding_addr_tx = UART0_HOLDNG_ADDRESS_TX;
            ie_addr_rx      = UART0_IER_ADDRESS_RX;
            serial_tx       = serial_uart_tx;
    }
}
/******************** End Serial UART SETUP ***********************************/


volatile serial_debug_t sd;


static void serial_initialize_rx_linked_list_descriptor(void *holding_reg_addr)
{
    // TODO - If this list gets any bigger, create a clever for loop
    pRxLinkedListDesc[0].mbr_nda = (uint32_t)&pRxLinkedListDesc[1];
    pRxLinkedListDesc[0].mbr_sa = (uint32_t)holding_reg_addr;
    pRxLinkedListDesc[0].mbr_da = (uint32_t)&rx_frame_buffers[0];
    pRxLinkedListDesc[0].mbr_ubc.blockDataLength = SERIAL_FRAME_SIZE;
    pRxLinkedListDesc[0].mbr_ubc.nextDescriptorControl.fetchEnable= 1;
    pRxLinkedListDesc[0].mbr_ubc.nextDescriptorControl.sourceUpdate = 1;
    pRxLinkedListDesc[0].mbr_ubc.nextDescriptorControl.destinationUpdate = 0;
    pRxLinkedListDesc[0].mbr_ubc.nextDescriptorControl.view= 1;


    pRxLinkedListDesc[1].mbr_nda = (uint32_t)&pRxLinkedListDesc[2];
    pRxLinkedListDesc[1].mbr_sa = (uint32_t)holding_reg_addr;
    pRxLinkedListDesc[1].mbr_da = (uint32_t)&rx_frame_buffers[1];
    pRxLinkedListDesc[1].mbr_ubc.blockDataLength = SERIAL_FRAME_SIZE;
    pRxLinkedListDesc[1].mbr_ubc.nextDescriptorControl.fetchEnable = 1;
    pRxLinkedListDesc[1].mbr_ubc.nextDescriptorControl.sourceUpdate = 1;
    pRxLinkedListDesc[1].mbr_ubc.nextDescriptorControl.destinationUpdate= 0;
    pRxLinkedListDesc[1].mbr_ubc.nextDescriptorControl.view = 1;
    
    pRxLinkedListDesc[2].mbr_nda = (uint32_t)&pRxLinkedListDesc[0];
    pRxLinkedListDesc[2].mbr_sa = (uint32_t)holding_reg_addr;
    pRxLinkedListDesc[2].mbr_da = (uint32_t)&rx_frame_buffers[2];
    pRxLinkedListDesc[2].mbr_ubc.blockDataLength = SERIAL_FRAME_SIZE;
    pRxLinkedListDesc[2].mbr_ubc.nextDescriptorControl.fetchEnable = 1;
    pRxLinkedListDesc[2].mbr_ubc.nextDescriptorControl.sourceUpdate = 1;
    pRxLinkedListDesc[2].mbr_ubc.nextDescriptorControl.destinationUpdate= 0;
    pRxLinkedListDesc[2].mbr_ubc.nextDescriptorControl.view = 1;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */


/**
 *  \brief Callback function for DMA receiving.
 *  \note  called from ISR
 */
static void serial_dma_callback_rx(XDMAC_TRANSFER_EVENT event, uintptr_t context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // FIXME handle context
    (void) context;
    sd.serial_rx_interrupts++;
    
    if(event == XDMAC_TRANSFER_COMPLETE) {
        xSemaphoreGiveFromISR(rx_semaphores[dma_head], &xHigherPriorityTaskWoken);
        dma_head = (dma_head + 1) % RX_DMA_BUFFERS;
        if(sd.has_overrun_err) {
            sd.has_overrun_err = false;
            ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR);
        }
    } else {
        ERROR_SET(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR, NULL);
        sd.has_overrun_err = true;
    }
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     should be performed to ensure the interrupt returns directly to the highest
     priority task.  The macro used for this purpose is dependent on the port in
     use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 *  \brief Callback function for DMA transmitting.
 */
static void serial_dma_callback_tx(XDMAC_TRANSFER_EVENT event, uintptr_t context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // FIXME handle context and error conditions
    (void) event;
    (void) context;
    sd.serial_tx_interrupts++;
    xSemaphoreGiveFromISR(txDoneMsgSemaphore, &xHigherPriorityTaskWoken);
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     should be performed to ensure the interrupt returns directly to the highest
     priority task.  The macro used for this purpose is dependent on the port in
     use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * Stop the serial RX interface and clear any errors
 */
static void serial_rx_stop()
{
    /* Stop the DMA first */
    XDMAC_ChannelDisable(dma_channel_rx);
}

/**
 * Checks for valid Start of Frame (SOF) & CRC checks the message
 * 
 * @param pFrame pointer to  the frame
 * @return 0 if frame is valid, non-zero otherwise
 */
static int serial_frame_error_check(const Serial_Frame_t *pFrame)
{
    int err = 0;
    uint16_t sof = pFrame->data[0] & SERIAL_SOF_MASK;
        /* Check for correct Start of Frame */
    if(sof != SERIAL_SOF) {
        err = __LINE__;
        ERROR_SET(TC_EMCY_REG_COM, ERROR_CODE_COMM_FRAME_ERROR, NULL);
        sd.has_frame_err = true;
        TraceE3(TrcMsgErr3, "SOF ERROR - %08x:%08x:%08x%02x",
            *((uint32_t *)&pFrame->data[0]),
            *((uint32_t *)&pFrame->data[4]),
            *((uint32_t *)&pFrame->data[8]),
            pFrame->data[12], 0,0);
    } else if(sd.has_frame_err) {
        sd.has_frame_err = false;
        ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_FRAME_ERROR);
        ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR);
        TraceE3(TrcMsgErr3, "Frame ERROR cleared", 0,0,0,0,0,0);
    }

    
    if(!err) {
        uint16_t crc = 0; /* CRC on the first 11 bytes */
        /* Calculate CRC - Frame length - 2 for the 16bit CRC */
        for(int i=0; i < SERIAL_FRAME_SIZE - 2; i++) {
            crc = update_crc_16(crc, pFrame->data[i]);
        }
        if(crc != pFrame->frame.crc) {
            err = __LINE__;
            ERROR_SET(TC_EMCY_REG_COM, ERROR_CODE_COMM_CRC_ERROR, NULL);
            sd.has_crc_err = true;
            TraceE3(TrcMsgErr3, "CRC ERROR - %08x:%08x:%08x%02x",
                *((uint32_t *)&pFrame->data[0]),
                *((uint32_t *)&pFrame->data[4]),
                *((uint32_t *)&pFrame->data[8]),
                pFrame->data[12], 0,0);
        } else if(sd.has_crc_err) {
            sd.has_crc_err = false;
            ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_CRC_ERROR);
            TraceE3(TrcMsgErr3, "CRC ERROR cleared", 0,0,0,0,0,0);
        }
    }        
     
    return err;
}

/*
 * Enable RX interrupt
 * 
 */
static void serial_ie_rx_enable()
{
    *((uint32_t*)ie_addr_rx) |= (US_IER_USART_RXRDY_Msk);
}

/*
 * Disable RX interrupt
 * 
 */
static void serial_ie_rx_disable()
{
    *((uint32_t*)ie_addr_rx) &= ~(US_IER_USART_RXRDY_Msk);
}

static void serial_task_rx(void *pvParameters) {
    int err;
    Serial_Frame_t *pFrame = NULL;
    uint16_t first_three_bit;     /* First three bits after the SOF */
   
    
    while (1) {
        err = 0;
        
        SemaphoreHandle_t sem = rx_semaphores[dma_tail];
        pFrame = &rx_frame_buffers[dma_tail];
        
        xSemaphoreTake(sem, portMAX_DELAY);
        sd.serial_rx_messages++;
        
        /**
         * A counting semaphore is used here.  If ever the value is not equal
         * to zero, the RX task missed a message. Each RX Buffer has a semaphore.
         * The interrupt handler gives the semaphore (increments it), this task
         * takes it (decrements it).  If this task decrements it and it is not
         * equal to zero, then the interrupt handler incremented it twice before
         * this task got to it.  In that case this message should be skipped
         * until the tail comes back around to the head.   This ensures the
         * messages are handled in order, despite one (or more) being dropped.
         */
        UBaseType_t sem_count = uxSemaphoreGetCount(sem);
        if(sem_count != 0) {
            // An overrun has occurred. Set and error and move on.
            ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR);
            // set rx to tx, the dropped packet is gone */
            sd.has_overrun_err = true;
            // Skip this message - it was dropped
            err = __LINE__;
            TraceE3(TrcMsgErr3, "Overrun ERROR - %08x:%08x:%08x%02x",
                    *((uint32_t *)&pFrame->data[0]),
                    *((uint32_t *)&pFrame->data[4]),
                    *((uint32_t *)&pFrame->data[8]),
                    pFrame->data[12], 0,0);
        } else if(sd.has_overrun_err) {
            sd.has_overrun_err = false;
            ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR);
            TraceE3(TrcMsgErr3, "Overrun ERROR cleared", 0,0,0,0,0,0);
        }
        /* Check start of frame */
        if(!err) {
            err = serial_frame_error_check(pFrame);          
            /* 
             * If there's an error, our frame has gotten out of whack.
             *  Need to resynchronize.
             */
            if(err){
                /* Stop DMA, stop task and exit function */
                serial_rx_stop();
                serial_ie_rx_enable();
                vTaskDelete(NULL);
                return;
            }
        }
        /* Pass the message on to the CAN Open Stack */
        can_msg_t co_msg;
        if(!err) {
            first_three_bit = pFrame->data[0] & ~(SERIAL_SOF_MASK);
            co_msg.identifier = first_three_bit << 8 | pFrame->data[1];
            co_msg.dlc = SERIAL_DATA_SIZE;
            memcpy(co_msg.data, pFrame->frame.data, sizeof(co_msg.data));
        }
        if(!err) {
            /* Send the message along */
            BaseType_t xStatus = xQueueSend(xCoQueue, &co_msg, 0);
            if(xStatus != true) {
                err = __LINE__;
                ERROR_SET(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR, NULL);
                sd.has_overrun_err = true;
            } else if(sd.has_overrun_err) {
                sd.has_overrun_err = false;
                ERROR_CLEAR(TC_EMCY_REG_COM, ERROR_CODE_COMM_OVERRUN_ERROR);
            }
        }
        dma_tail = (dma_tail + 1) % RX_DMA_BUFFERS;
    }
}

/**
 * Returns the current resync status
 * @return 
 */
int serial_task_resync_status_get(void)
{
    return frame_resynced;
}


/**
 * Reset the resync status
 * @return 
 */
void serial_task_resync_status_reset(void)
{
    frame_resynced = 0;
}

/**
 * Resynchronize the serial comms when an error is detected on the bus
 * @return 
 */
static inline void serial_resync(void)
{
    static Serial_Frame_t resync_frame;
    static uint8_t buffer[32] = {0}; /* Must be a power of 2 */
    static int head = 0;
    static int tail = 0;
    const static int buffer_size = sizeof(buffer);
    
    int size = 0;
    
    /* First read everything available */
    while (usart_regs->US_CSR & US_CSR_USART_RXRDY_Msk) {
        tail = (tail + 1) & (buffer_size - 1);
        buffer[tail] = (usart_regs->US_RHR & US_RHR_RXCHR_Msk);
        if(tail == head) {
            /* do not let the snake eat its tail */
            head = (head + 1) & (buffer_size - 1);
        }
    }
    /* Now see if the last 13 (frame size) bytes are a good message */
    /* Only the last 13 (frame size) matter, so move the head forward */
    size = tail < head ? (tail + buffer_size) - head : tail - head;
    size++;  /* inclusive in both cases ^^ */
    if(size >= SERIAL_FRAME_SIZE) {
        head = ((size - SERIAL_FRAME_SIZE) + head) & (buffer_size - 1);
        size = SERIAL_FRAME_SIZE;
    }
    /* now see if you have a good frame */
    if(size == SERIAL_FRAME_SIZE && (buffer[head] & SERIAL_SOF_MASK) == SERIAL_SOF) {
        uint16_t crc = 0; /* CRC on the first 11 bytes */
        int temp_head = head;
        /* Calculate CRC - Frame length - 2 for the 16bit CRC */
        for(int i=0; i < SERIAL_FRAME_SIZE; i++) {
            resync_frame.data[i] = buffer[temp_head];
            if(i < SERIAL_FRAME_SIZE - 2) {
                crc = update_crc_16(crc, resync_frame.data[i]);
            }
            temp_head = (temp_head + 1) & (buffer_size - 1);
        }
        if(crc == resync_frame.frame.crc) {
            /* Send the message along */
            xQueueSend(xCoQueue, &resync_frame, 0);
            frame_resynced = 1;
            tail = 0;
            head = 0;
            serial_ie_rx_disable();
            serial_start_rx();
        } else {
            /* just move the head forward and wait */
            head = (head + 1) & (buffer_size - 1);
        }
    }
    TraceDbg(TrcMsgDbg, "buffer head:%x tail:%x size:% d",
            buffer[head], buffer[tail], size, 0,0,0);
}

/**
 * Initially we used the UART for serial coms, which meant for RS-485 we
 * had to make our own RTS.   This is here to support those older boards
 */
static inline void serial_uart_tx(void)
{
    /* Enable TX */
    RS485_RTS_Set();
    /* Send it */
    XDMAC_ChannelTransfer(dma_channel_tx, &tx_frame_buffer,
            holding_addr_tx, SERIAL_FRAME_SIZE );
    /* Wait for it */
    xSemaphoreTake(txDoneMsgSemaphore, portMAX_DELAY);
    /* Make sure the UART has completely finished sending */
    while( !(UART0_REGS->UART_SR & UART_IMR_TXEMPTY_Msk) );
    /* Pull the Request to Send line low */
    RS485_RTS_Clear();
}

void USART1_Handler( void )
{
    if(USART1_REGS->US_CSR & US_IER_USART_TIMEOUT_Msk) {
        USART1_REGS->US_IER &= ~US_IER_USART_TIMEOUT(1);
        USART1_REGS->US_RTOR = 0;
        USART1_REGS->US_CR |= US_CR_USART_STTTO(1);
        
        XDMAC_ChannelTransfer(dma_channel_tx, &tx_frame_buffer,
                holding_addr_tx, SERIAL_FRAME_SIZE );

        TestPoint_PA4_Set();
    } else if(USART1_REGS->US_CSR & US_IER_USART_RXRDY_Msk ) {
        serial_resync();
    } else {
        TestPoint_PA5_Toggle();
        TestPoint_PA5_Toggle();
    }
}


void USART2_Handler( void )
{
    if(USART2_REGS->US_CSR & US_IER_USART_RXRDY_Msk ) {
        serial_resync();
    }
}


void UART0_Handler( void )
{
    if(UART0_REGS->UART_SR & UART_IER_RXRDY_Msk ) {
        serial_resync();
    }
}


/**
 * Send a message without the manual RTS
 */
static inline void serial_usart_rs485_tx(void)
{
    TestPoint_PA4_Clear();
    USART1_REGS->US_RTOR  = US_RTOR_TO(30);
    USART1_REGS->US_CR   |= US_CR_USART_RETTO(1);
    USART1_REGS->US_IER  |= US_IER_USART_TIMEOUT(1);
    xSemaphoreTake(txDoneMsgSemaphore, portMAX_DELAY);   
}

/**
 * Send a message without the manual RTS
 */
static inline void serial_usart_rs422_tx(void)
{
    XDMAC_ChannelTransfer(dma_channel_tx, &tx_frame_buffer,
            holding_addr_tx, SERIAL_FRAME_SIZE );
    xSemaphoreTake(txDoneMsgSemaphore, portMAX_DELAY);
}

static void serial_task_tx(void *pvParameters)
{
    BaseType_t xStatus = pdPASS;
    
    while (1) {
        xStatus = xQueueReceive(xTxQueue, &tx_frame_buffer, xRxTxTicksToWait);
        if (xStatus == pdTRUE) {
            tx_frame_buffer.data[0] &= ~SERIAL_SOF_MASK;
            tx_frame_buffer.data[0] |= SERIAL_SOF;
            
            uint16_t crc = 0;
            for(int i=0; i < 11; i++) {
                crc = update_crc_16(crc, tx_frame_buffer.data[i]);
            }
            tx_frame_buffer.frame.crc = crc;
#if 0 /* This is a self-perpetuating message, only use if absolutely necessary */
            TraceDbg(TrcMsgSerial, "%08x:%08x:%08x%02x",
                    *((uint32_t *)&tx_frame_buffer.data[0]),
                    *((uint32_t *)&tx_frame_buffer.data[4]),
                    *((uint32_t *)&tx_frame_buffer.data[8]),
                    tx_frame_buffer.data[12], 0,0);
#endif
            serial_tx();
            sd.serial_tx_messages++;
        }
    }
}


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/**
 * Send a message to the external interface through the serial port
 * @param cobid CAN Open COBID
 * @param data pointer to data
 * @param dlc Data Length Code
 */
void serial_task_send(uint16_t cobid, uint8_t *data, uint8_t dlc)
{
    BaseType_t xStatus = pdPASS;
    Serial_Frame_t frame;
    uint8_t *psend_data = NULL; // point to proper place in 485 frame

    frame.frame.cob_id = ((cobid << 8) & 0xFF00) | ((cobid >> 8) & 0xFF);
    frame.frame.control = dlc;
    psend_data = frame.frame.data;

    /* Copy 'Data Length Code' bytes from data buffer */
    for(uint32_t i = 0; i < dlc; i++) {
        psend_data[i] = data[i];
    }
    /* Zero fill the rest */
    for(uint32_t i = dlc; i < SERIAL_DATA_SIZE; i++) {
        psend_data[i] = 0;
    }
    xStatus = xQueueSend(xTxQueue, &frame, 0);
    if(xStatus != pdTRUE) {
        // FIXME handle this error
    }
}

/**
 * Send CAN Open Message to external interface
 * 
 * @param Identifier CAN message identifier 
 * @param Data CAN message Data (payload) 
 * @param DLC CAN message data length code (DLC)
 */
void serial_ext_msg_snd(uint32_t identifier, uint8_t *data, uint8_t dlc)
{
    serial_task_send((uint16_t) identifier, data, dlc);
}

/**
 * Received a Message from external interface
 * 
 * Blocks until a message is received
 * 
 * @param Identifier CAN message identifier 
 * @param Data CAN message Data (payload) 
 * @param DLC CAN message data length code (DLC)
 * @param block zero for non-blocking mode, non-zero for blocking
 * @return 1 for message received, -1 for error, 0 when non-blocking and no message is ready
 */
int serial_ext_msg_rcv(uint32_t *identifier, uint8_t *data, uint8_t *dlc, int block)
{
    int err = 0;
    can_msg_t co_msg;
    
    TickType_t delay = block ? portMAX_DELAY:0;
    
    BaseType_t xStatus = xQueueReceive(xCoQueue, &co_msg, delay);
    LED_GREEN_Toggle();
    if(xStatus == false) {
        /* Only an err if in blocking mode */
        if(block) {
            err = -1;
        }
    } else {
        err = 1;
        *identifier = co_msg.identifier;
        *dlc = sizeof(co_msg.data);
        memcpy(data, co_msg.data, *dlc);
    }
    return err;
}

/**
 * Provide a Queue to listen to incoming SDO requests from client
 * @param pxOQueue pointer to SDO queue
 */
void serial_task_SDO_queue_set(QueueHandle_t *pxOQueue)
{
    pxSDOQueue = pxOQueue;
}

/**
 * Com reset handler - ISR
 * This is called when the user asserts the comm reset pin
 * This should clear an com errors and reset the DMA RX 
 * @param pin  Which pin is asserted
 * @param context optional context pointer passed into Harmony library callback
 */
void serial_reset_control(PIO_PIN pin, uintptr_t context)
{
    if(ComReset_Get() == COMM_RESET_STATE) {
        /* Disable interrupts so we do not get it again before we are ready */
        PIO_PinInterruptDisable(ComReset_PIN);
        
        serial_rx_stop();
        
        vTaskDelete(serial_task_rx_handle);
        APP_restart_serial_rx();
    }
}

/**
 * Enable the COM Reset Interrupt
 */
static void serial_reset_enable_interrupts()
{
    PIO_PinInterruptCallbackRegister(ComReset_PIN, serial_reset_control, (uintptr_t)NULL);
    PIO_PinInterruptEnable(ComReset_PIN);
}

/**
 * Clear any pending characters in the buffer so the serial task can start with
 * a fresh frame.
 * 
 * @note UART is not implemented here, it has been deprecated in favor of USARTS
 * and only exists on old dev boards.
 * 
 * @return 0 on success, none zero otherwise
 */
static int serial_clear_pending(void)
{
    size_t processed_size = 0;
    uint32_t err = 0;
    
    /* Make sure usart_regs has been set (the config function called), if not
     * it will be NULL and this will crash.  */
    while (usart_regs != NULL && usart_regs->US_CSR & US_CSR_USART_RXRDY_Msk) {
        char buff[2] = {0,0};
        /* Read error status */
        err = (usart_regs->US_CSR & (US_CSR_USART_OVRE_Msk | US_CSR_USART_FRAME_Msk | US_CSR_USART_PARE_Msk));
        
        if (usart_regs->US_MR & US_MR_USART_MODE9_Msk) {
            *((uint16_t*) buff) = (usart_regs->US_RHR & US_RHR_RXCHR_Msk);
            
        } else {
            *buff = (usart_regs->US_RHR & US_RHR_RXCHR_Msk);
        }
        
        processed_size++;
        TraceE3(TrcMsgDbg, "buff %08x:%08x cleared:%d err:%d",
                buff[0], buff[1], processed_size, err,0,0);
    }
    TraceDbg(TrcMsgDbg, "cleared:%d err:%d", processed_size, err,0,0,0,0);
    return err;
}

/**
 * Start/Restart the Serial RX task
 * 
 * The Serial RX task is deleted when the COM reset is asserted.  The main
 * app thread is responsible for call this function.  
 * 
 * Do not confuse this with init.   This is called from the original init
 * function on system startup.   It should only be called after that,
 * when the original task was delete (because of a COM Reset)
 * 
 */
void serial_start_rx(void)
{
    taskENTER_CRITICAL();
    /* Reset head and tail pointers */
    dma_head = 0;
    dma_tail = 0;
    
    serial_clear_pending();

    /* Made sure all semaphores are zero */
    for(int i = 0; i < RX_DMA_BUFFERS; i++) {
         while( xSemaphoreTake(rx_semaphores[i], 0) == pdTRUE) ;
    }
    
    /* Create Rx Task */
    serial_task_rx_handle = xTaskCreateStatic(serial_task_rx, "Serial Rx task",
            SERIAL_TASK_STACK_SIZE, 0, SERIAL_TASK_RX_PRIORITY, rxStack, &rxTaskBuffer);
    
    /* Setup the callback */
    XDMAC_ChannelCallbackRegister(dma_channel_rx, serial_dma_callback_rx,
            (uintptr_t)NULL);
    
    /* Setup RX DMA Chain */
    serial_initialize_rx_linked_list_descriptor(holding_addr_rx);
    XDMAC_ChannelLinkedListTransfer(dma_channel_rx,
            (uint32_t)&pRxLinkedListDesc[0], &rxFirstDescriptorControl);
    
    taskEXIT_CRITICAL();
    
    /* Now start looking for COM Reset again */
    serial_reset_enable_interrupts();
}

bool serial_has_err(void)
{
    return SERIAL_HAS_COM_ERROR(sd);
}

/**
 * @Function
 * int serial_task_init(void)
 * @Summary
 *     Initialize serial tasks.
 * @Remarks
 *     Refer to the serial_task.h interface header for function usage details.
 * @return 0 on success, error otherwise
 */
int serial_task_init(serial_device_t *dev)
{
    serial_dev_setup(dev);
    memset((void*)&sd, 0, sizeof(sd));
    
    /* Create the static queues */
    xTxQueue = xQueueCreateStatic( BUFFER_COUNT_TX, QUEUE_ITEM_SIZE_TX,
            ucTxQueueStorageArea, &xTxStaticQueue );
    xCoQueue = xQueueCreateStatic( BUFFER_COUNT_CO, QUEUE_ITEM_SIZE_CO,
            ucCoQueueStorageArea, &xCoStaticQueue );
    
    /* Create the RX semaphores */
    for(int i = 0; i < RX_DMA_BUFFERS; i++) {
         rx_semaphores[i]= xSemaphoreCreateCountingStatic(
                 COUNT_SEM_RX_MAX, 0, &rx_semaphores_buffers[i]);
    }
    /* Create the TX semaphore */
    txDoneMsgSemaphore = xSemaphoreCreateBinaryStatic(&txSemaphoreBuffer);

    /* Create Tx Task */
    serial_task_tx_handle = xTaskCreateStatic(serial_task_tx, "Serial Tx task",
            SERIAL_TASK_STACK_SIZE, 0, SERIAL_TASK_TX_PRIORITY, txStack, &txTaskBuffer);
    
    /* Call the RX startup function */
    serial_start_rx();
    
    XDMAC_ChannelCallbackRegister(dma_channel_tx, serial_dma_callback_tx,
            (uintptr_t)NULL);
    
    serial_reset_enable_interrupts();
    
    return 0;
}

void serial_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, serial_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);    
    fh_fault_handlers_register(ERROR_CODE_COMM_OVERRUN_ERROR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_COMM_FRAME_ERROR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_COMM_CRC_ERROR, FH_ALERT);
}


static void* serial_error_detail_strategy(void* arg)
{
    if(arg != NULL){
        base_error_detail_t* b = (base_error_detail_t*)arg;
        int index = b->e_cnt  & (SIZEOF_ARRAY(error_detail) - 1);
        memcpy(&error_detail[index], arg, sizeof(serial_error_detail_t));
    }
    
    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}


/* *****************************************************************************
 End of File
 */
