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
/**
 * @Company
 *      Exoterra
 * @File
 *      mcan_task.c
 * @Summary
 *      MCAN interface & tasks
 * @Description
 *      This files contains all the logic to interfaces between the MCAN system
 *      and our application.  There are several tasks running here that are
 *      responsible for receiving/transmitting/queuing CAN messages.
 * 
 */
#include "thruster_control.h"
#include "mcan_task.h"                  // Header for this module
#include "include/mcan_p.h"             // Private MCAN module include
#include "trace/trace.h"      // Trace Messages
#include "macro_tools.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

uint8_t Can0MessageRAM[MCAN0_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32)))__attribute__((space(data), section (".ram_nocache")));
uint8_t Can1MessageRAM[MCAN1_MESSAGE_RAM_CONFIG_SIZE] __attribute__((aligned (32)))__attribute__((space(data), section (".ram_nocache")));

/************************* RTOS definitions ***********************************/
static TaskHandle_t xTaskRx0Fifo = NULL;
static TaskHandle_t xTaskRx1Fifo = NULL;

#define rxFifo_TASK_STACK_SIZE    ( configMINIMAL_STACK_SIZE * 5 )
#define rxFifo_TASK_PRIORITY      ( tskIDLE_PRIORITY )

/* Buffer for the task being to use as its stack. */
static StackType_t rxFifo0Stack[rxFifo_TASK_STACK_SIZE];
static StackType_t rxFifo1Stack[rxFifo_TASK_STACK_SIZE];

/* Structure that will hold the TCB of the task being created. */
static StaticTask_t rxFifo0TaskBuffer;
static StaticTask_t rxFifo1TaskBuffer;

static SemaphoreHandle_t xMCAN0Semaphore;
static StaticSemaphore_t xMCAN0SemaphoreBuffer;
static SemaphoreHandle_t xMCAN1Semaphore;
static StaticSemaphore_t xMCAN1SemaphoreBuffer;

#define TX_TIMEOUT_MS      (10)

/* Can Open Queue Item size */
#define QUEUE_ITEM_SIZE_CO  (sizeof(Mailbox8Type))
#define BUFFER_COUNT_CO     5
#define RX_TX_WAIT_TICKS    1000 /* How long to wait for rcv to clear */
static  QueueHandle_t       xCoQueue;
static  StaticQueue_t       xCoStaticQueue;
static uint8_t ucCoQueueStorageArea[QUEUE_ITEM_SIZE_CO * BUFFER_COUNT_CO];

range_node_t *can_id_tree = NULL;


/************************* End RTOS definitions *******************************/

/* MCAN state enum */
typedef enum
{
    MCAN0_CONTEXT_RX_FIFO_0,
    MCAN0_CONTEXT_RX_FIFO_1,
    MCAN0_CONTEXT_RX_BUF,
    MCAN1_CONTEXT_RX_FIFO_0,
    MCAN1_CONTEXT_RX_FIFO_1,
    MCAN1_CONTEXT_RX_BUF,
    MCAN0_CONTEXT_TX,
    MCAN1_CONTEXT_TX,
    MCAN_INVALID
} MCAN_CONTEXT;

typedef enum
{
	CAN_RX_FIFO_0 = 0,
	CAN_RX_FIFO_1 = 1
} MCan_FifoType;

/* Below is a callback function and structure for holding information about
 * which bus the caller is going to transmit on.  After initialization, this
 * is transparent to the caller.  It is specified during initialization.  The
 * rest of the application just calls the generic TX functions.
 */
typedef bool (*MCAN_TX) (uint32_t id, uint8_t length, uint8_t* data,
                            MCAN_MODE mode, MCAN_MSG_TX_ATTRIBUTE msgAttr);

typedef struct {
    /* Which Tx function MCAN0 or MCAN1 */
    MCAN_TX ptx;
    /* The semaphore tied to the MCAN Tx */
    SemaphoreHandle_t *psem;  
} mcan_tx_info_t;

/* Off by default */
static mcan_tx_info_t mcan_tx_client = {0, 0};
static mcan_tx_info_t mcan_tx_extern = {0, 0};

/**
 * ISR Callback for RX 
 * @param context context pointer provided to the mcan driver when the callback
 * was registered
 */
void mcan_task_ISR_callback_rx(uintptr_t context)
{
    MCAN_ERROR status = MCAN_ERROR_NONE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t *pnotify = NULL;
    
    switch(context) {
        case MCAN0_CONTEXT_RX_FIFO_0:
            status = MCAN0_ErrorGet();
            pnotify = &xTaskRx0Fifo;
            break;
        case MCAN1_CONTEXT_RX_FIFO_0:
            status = MCAN1_ErrorGet();
            pnotify = &xTaskRx0Fifo;
            break;
        case MCAN0_CONTEXT_RX_FIFO_1:
            status = MCAN0_ErrorGet();
            pnotify = &xTaskRx1Fifo;
            break;
        case MCAN1_CONTEXT_RX_FIFO_1:
            status = MCAN1_ErrorGet();
            pnotify = &xTaskRx1Fifo;
            break;
        default:
            // Invalid context
            break;
    }    
    if( pnotify && (((status & MCAN_PSR_LEC_Msk) == MCAN_ERROR_NONE) || 
                 ((status & MCAN_PSR_LEC_Msk) == MCAN_PSR_LEC_NO_CHANGE))) {
        vTaskNotifyGiveFromISR(*pnotify, &xHigherPriorityTaskWoken);
    } else {
        // FIXME handle errors
    }
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     should be performed to ensure the interrupt returns directly to the highest
     priority task.  The macro used for this purpose is dependent on the port in
     use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * ISR Callback for TX 
 * @param context context pointer provided to the mcan driver when the callback
 * was registered
 */
void mcan_task_ISR_callback_tx(uintptr_t context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    mcan_registers_t* mcan_regs = NULL;
    
    SemaphoreHandle_t *psem = NULL;
    
    switch(context) {
        case MCAN0_CONTEXT_TX:
            psem = &xMCAN0Semaphore;
            mcan_regs = MCAN0_REGS;
            break;
        case MCAN1_CONTEXT_TX:
        default:
            psem = &xMCAN1Semaphore;
            mcan_regs = MCAN1_REGS;
            break;
    }
    
    
    /* Give the remaining semaphores */
    uint32_t sem_cnt = uxSemaphoreGetCount(*psem);
    uint32_t free_msgs = mcan_regs->MCAN_TXFQS & MCAN_TXFQS_TFFL_Msk;
    int32_t num_sems_to_give = (free_msgs - sem_cnt);

    TraceDbg(TrcMsgDbg, "fms:%d sc:%d nstg:%d",
            free_msgs, sem_cnt, num_sems_to_give, 0,0,0);

    for(int i = 0; (i < num_sems_to_give) && num_sems_to_give > 0 && psem; i++) {

        TraceDbg(TrcMsgDbg, "ns:%d fms:%d scc:%d sci:%d nstg:%d",
                i, free_msgs, uxSemaphoreGetCount(*psem), sem_cnt,num_sems_to_give,0);

        BaseType_t tmp = pdFALSE;
        BaseType_t sem_status = xSemaphoreGiveFromISR(*psem, &tmp);
        if(sem_status != pdTRUE){
            /* Fix me - Add error*/
        }
        if(tmp != pdFALSE){
            xHigherPriorityTaskWoken = tmp;
        }
    }
    
    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
     *      should be performed to ensure the interrupt returns directly to the highest
     *      priority task.  The macro used for this purpose is dependent on the port in
     *      use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 *  * Get an mcan message and store it in the provided mailbox
 * @param fifo which fifo 0 or 1
 * @param pmailbox pointer to mailbox
 * @return Number of outstanding messages
 */
static uint32_t mt_rx_fifo_msg_mcan1_get(MCan_FifoType fifo, Mailbox8Type *pmailbox) {
    uint32_t get_index = 0;
    uint32_t fill_level = 0;
    uint32_t fifo_ack_val;
    uint32_t *fifo_ack_reg;

    mcan_rxf0e_registers_t *rxfeFifo = NULL;

    uint8_t msg_length[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

    if (fifo == CAN_RX_FIFO_0) {
        get_index = (MCAN1_REGS->MCAN_RXF0S & MCAN_RXF0S_F0GI_Msk) >> MCAN_RXF0S_F0GI_Pos;
        fill_level = (MCAN1_REGS->MCAN_RXF0S & MCAN_RXF0S_F0FL_Msk) >> MCAN_RXF0S_F0FL_Pos;
        rxfeFifo = (mcan_rxf0e_registers_t *)
                (Can1MessageRAM + (get_index * MCAN1_RX_FIFO0_ELEMENT_SIZE));
        fifo_ack_reg = (uint32_t *) & MCAN1_REGS->MCAN_RXF0A;
        fifo_ack_val = MCAN_RXF0A_F0AI(get_index);

    } else if (fifo == CAN_RX_FIFO_1) {
        get_index = (MCAN1_REGS->MCAN_RXF1S & MCAN_RXF1S_F1GI_Msk) >> MCAN_RXF1S_F1GI_Pos;
        fill_level = (MCAN1_REGS->MCAN_RXF1S & MCAN_RXF1S_F1FL_Msk) >> MCAN_RXF1S_F1FL_Pos;
        rxfeFifo = (mcan_rxf0e_registers_t *)
                (Can1MessageRAM + MCAN1_RX_FIFO1_SIZE +
                (get_index * MCAN1_RX_FIFO1_ELEMENT_SIZE));
        fifo_ack_reg = (uint32_t *) & MCAN1_REGS->MCAN_RXF1A;
        fifo_ack_val = MCAN_RXF1A_F1AI(get_index);
    }
    if (fill_level > 0) {
        /* Get received identifier */
        if (rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_XTD_Msk) {
            pmailbox->info.id = rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_ID_Msk;
        } else {
            // Standard Identifier is 11 bits (0x7FF)
            pmailbox->info.id = (rxfeFifo->MCAN_RXF0E_0 >> 18) & 0x7FF;
        }

        /* Check RTR and FDF bits for Remote/Data Frame */
        if ((rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_RTR_Msk) && ((rxfeFifo->MCAN_RXF0E_1 & MCAN_RXF0E_1_FDF_Msk) == 0)) {
            pmailbox->info.attr = MCAN_MSG_RX_REMOTE_FRAME;
        } else {
            pmailbox->info.attr = MCAN_MSG_RX_DATA_FRAME;
        }

        /* Get received data length */
        pmailbox->info.length = msg_length[((rxfeFifo->MCAN_RXF0E_1 & MCAN_RXF0E_1_DLC_Msk) >> MCAN_RXF0E_1_DLC_Pos)];

        /* Copy data to user buffer */
        memcpy(pmailbox->data, (uint8_t *) & rxfeFifo->MCAN_RXF0E_DATA, pmailbox->info.length);

        /* Ack the fifo position */
        *fifo_ack_reg = fifo_ack_val;
    }
    return fill_level;
}
/**
 * It pains me to do this.  This function is a copy of the other mcan with
 * just the names changes.  I could not think of a more elegant way to
 * switch between the MCAN0 and MCAN1 registers and pointers.  Quickly anyway.
 * There must be a better way. This needs work.
 
 * @param fifo
 * @param pmailbox
 * @return 
 */
static uint32_t mt_rx_fifo_msg_mcan0_get(MCan_FifoType fifo, Mailbox8Type *pmailbox) {
    uint32_t get_index = 0;
    uint32_t fill_level = 0;
    uint32_t fifo_ack_val;
    uint32_t *fifo_ack_reg;

    mcan_rxf0e_registers_t *rxfeFifo = NULL;

    uint8_t msg_length[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

    if (fifo == CAN_RX_FIFO_0) {
        get_index = (MCAN0_REGS->MCAN_RXF0S & MCAN_RXF0S_F0GI_Msk) >> MCAN_RXF0S_F0GI_Pos;
        fill_level = (MCAN0_REGS->MCAN_RXF0S & MCAN_RXF0S_F0FL_Msk) >> MCAN_RXF0S_F0FL_Pos;
        rxfeFifo = (mcan_rxf0e_registers_t *)
                (Can0MessageRAM + (get_index * MCAN0_RX_FIFO0_ELEMENT_SIZE));
        fifo_ack_reg = (uint32_t *) & MCAN0_REGS->MCAN_RXF0A;
        fifo_ack_val = MCAN_RXF0A_F0AI(get_index);

    } else if (fifo == CAN_RX_FIFO_1) {
        get_index = (MCAN0_REGS->MCAN_RXF1S & MCAN_RXF1S_F1GI_Msk) >> MCAN_RXF1S_F1GI_Pos;
        fill_level = (MCAN0_REGS->MCAN_RXF1S & MCAN_RXF1S_F1FL_Msk) >> MCAN_RXF1S_F1FL_Pos;
        rxfeFifo = (mcan_rxf0e_registers_t *)
                (Can0MessageRAM + MCAN0_RX_FIFO1_SIZE +
                (get_index * MCAN0_RX_FIFO1_ELEMENT_SIZE));
        fifo_ack_reg = (uint32_t *) & MCAN0_REGS->MCAN_RXF1A;
        fifo_ack_val = MCAN_RXF1A_F1AI(get_index);
    }
    if (fill_level > 0) {
        /* Get received identifier */
        if (rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_XTD_Msk) {
            pmailbox->info.id = rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_ID_Msk;
        } else {
            // Standard Identifier is 11 bits (0x7FF)
            pmailbox->info.id = (rxfeFifo->MCAN_RXF0E_0 >> 18) & 0x7FF;
        }

        /* Check RTR and FDF bits for Remote/Data Frame */
        if ((rxfeFifo->MCAN_RXF0E_0 & MCAN_RXF0E_0_RTR_Msk) && ((rxfeFifo->MCAN_RXF0E_1 & MCAN_RXF0E_1_FDF_Msk) == 0)) {
            pmailbox->info.attr = MCAN_MSG_RX_REMOTE_FRAME;
        } else {
            pmailbox->info.attr = MCAN_MSG_RX_DATA_FRAME;
        }

        /* Get received data length */
        pmailbox->info.length = msg_length[((rxfeFifo->MCAN_RXF0E_1 & MCAN_RXF0E_1_DLC_Msk) >> MCAN_RXF0E_1_DLC_Pos)];

        /* Copy data to user buffer */
        memcpy(pmailbox->data, (uint8_t *) & rxfeFifo->MCAN_RXF0E_DATA, pmailbox->info.length);

        /* Ack the fifo position */
        *fifo_ack_reg = fifo_ack_val;
    }
    return fill_level;
}
/**
 * Task responsible for reading message from FIFO 0 of the mcan interfaces
 * 
 * @fixe currently this reads from both interfaces.  In future, there should
 * be a task for the external interface and one for the internal clients
 * 
 * @param pvParameters unused
 */
static void mt_rx_fifo0_task(void *pvParameters)
{
    xTaskRx0Fifo = xTaskGetCurrentTaskHandle();
    Mailbox8Type mailbox;
    uint32_t fifo_entries = 0;
    const TickType_t xRxTxTicksToWait = pdMS_TO_TICKS(RX_TX_WAIT_TICKS);
    
    // Turn on RX Fifo new message interrupts
    MCAN0_REGS->MCAN_IE |= MCAN_IE_RF0NE_Msk;
    MCAN1_REGS->MCAN_IE |= MCAN_IE_RF0NE_Msk;
    
    for (;;) {
        // Wait for notification that RX Fifo is available.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        do {
            fifo_entries = mt_rx_fifo_msg_mcan0_get(CAN_RX_FIFO_0, &mailbox);
            if (fifo_entries == 0) {
                fifo_entries = mt_rx_fifo_msg_mcan1_get(CAN_RX_FIFO_0, &mailbox);
            }
            if (fifo_entries == 0) {
                // Turn on RX Fifo new message interrupts
                MCAN0_REGS->MCAN_IE |= MCAN_IE_RF0NE_Msk;
                MCAN1_REGS->MCAN_IE |= MCAN_IE_RF0NE_Msk;
            } else {
                /* Currently this FIFO handles the CAN Open Stack only */
                BaseType_t xStatus = xQueueSend(xCoQueue, &mailbox, xRxTxTicksToWait);
                if(xStatus != true) {
                    // FIXME - Handle errors
                }
            }
        } while (fifo_entries >= 1);
    }
}

/**
 * Task responsible for reading message from FIFO 1 of the mcan interfaces
 * 
 * @fixe currently this reads from both interfaces.  In future, there should
 * be a task for the external interface and one for the internal clients
 * 
 * @param pvParameters unused
 */
static void mt_rx_fifo1_task(void *pvParameters)
{
    xTaskRx1Fifo = xTaskGetCurrentTaskHandle();
    Mailbox8Type mailbox;
    uint32_t fifo_entries = 0;
    
    // Turn on RX Fifo new message interrupts
    MCAN0_REGS->MCAN_IE |= MCAN_IE_RF1NE_Msk;
    MCAN1_REGS->MCAN_IE |= MCAN_IE_RF1NE_Msk;

    for (;;) {
        // Wait for notification that RX Fifo is available.
        BaseType_t xEvent = ulTaskNotifyTake(pdTRUE, 1000/portTICK_RATE_MS);
        if(xEvent) {
            do {
                fifo_entries = mt_rx_fifo_msg_mcan0_get(CAN_RX_FIFO_1, &mailbox);
                if (fifo_entries == 0) {
                    fifo_entries = mt_rx_fifo_msg_mcan1_get(CAN_RX_FIFO_1, &mailbox);
                }
                if (fifo_entries == 0) {
                    // Turn on RX Fifo new message interrupts
                    MCAN0_REGS->MCAN_IE |= MCAN_IE_RF1NE_Msk;
                    MCAN1_REGS->MCAN_IE |= MCAN_IE_RF1NE_Msk;
                } else {
                    range_node_t *n = binary_range_search(can_id_tree, mailbox.info.id);
                    if(n != NULL) {
                        mcan_msg_callback_t *cb = container_of(n, mcan_msg_callback_t, node);
                        cb->cb(mailbox.info.id, mailbox.data, mailbox.info.length);
                    }
                }
            } while (fifo_entries >= 1);
        }
        task_flags |= TASK_FLAG_CAN_INT;
    }
}

/**
 * Send CAN Message to external interface
 * 
 * @param Identifier CAN message identifier 
 * @param Data CAN message Data (payload) 
 * @param DLC CAN message data length code (DLC)
 * @param info pointers to TX function (which interface) and TX semaphore
 */
static int mcan_tx_mcan(uint32_t cobid, uint8_t *data, can_dlc_t dlc, mcan_tx_info_t *info)
{
    int err = 0;
    BaseType_t got_sem = pdFALSE;
    bool sent = false;
    int msec = 0; /* millisecond wait */

    while(got_sem == pdFALSE && msec < TX_TIMEOUT_MS) {
        taskENTER_CRITICAL();
        got_sem = xSemaphoreTake(*(info->psem), 0);

        if(got_sem != pdTRUE){
            err = __LINE__;
            TraceDbg(TrcMsgDbg, "MCAN fifo full, wait", 0,0,0,0,0,0);
        } else {

            sent = info->ptx(cobid, dlc, data, MCAN_MODE_NORMAL, MCAN_MSG_ATTR_TX_FIFO_DATA_FRAME);
            if(!sent) {
                err = __LINE__;
                TraceE2(TrcMsgErr2, "MCAN MSG ERR", 0,0,0,0,0,0);
            }
        }
        taskEXIT_CRITICAL();
        if(got_sem == pdFALSE) {
            TickType_t xNextWakeTime = xTaskGetTickCount();
            /* Place this task in the blocked state until it is time to run again. */
            vTaskDelayUntil(&xNextWakeTime, (1/portTICK_PERIOD_MS));
        }
        msec++;
    }
    if(got_sem == pdFALSE) {
        TraceE2(TrcMsgErr2, "ERROR: MCAN FIFO FULL", 0, 0, 0, 0,0,0);
    }
    return err;
}

/**
 * Send a message to the mcan clients
 * @param cobid CAN ID
 * @param data data to sent
 * @return 0 on success. non-zero otherwise
 */
int mcan_send_msg_client(uint16_t cobid, uint8_t *data, can_dlc_t dlc)
{
    return mcan_tx_mcan(cobid, data, dlc, &mcan_tx_client);
}

/**
 * Send a sync message to the mcan clients
 * @return 0 on success. non-zero otherwise.
 */
int mcan_send_sync_client(void)
{
    return mcan_tx_mcan(0x80, 0, CAN_DLC_0, &mcan_tx_client);
}

int mcan_send_msg_extern(uint32_t cobid, uint8_t *data, uint8_t dlc)
{
    return mcan_tx_mcan(cobid, data, dlc, &mcan_tx_extern);
}
/**
 * Received a Message from external interface
 * 
 * @note Maximum length of data message is 8, any other messages
 * will not be returned here
 * 
 * Blocks until a message is received
 * 
 * @param Identifier CAN message identifier 
 * @param Data CAN message Data (payload) 
 * @param DLC CAN message data length code (DLC)
 * @param block zero for non-blocking mode, non-zero for blocking
 * @return 1 for message received, -1 for error, 0 when non-blocking and no message is ready
 */
int mcan_ext_msg_rcv(uint32_t *identifier, uint8_t *data, uint8_t *dlc, int block)
{
    int err = 0;
    Mailbox64Type co_msg; /* 64 because the can driver can receive > 8 */
    
    TickType_t delay = block ? portMAX_DELAY:0;
    
    BaseType_t xStatus = xQueueReceive(xCoQueue, &co_msg, delay);
    if(xStatus == false) {
        /* Only an err if in blocking mode */
        if(block) {
            err = -1;
        }
    } else {
        /* 8 byte or  messages only */
        err = 1;
        if(co_msg.info.length <= CAN_DLC_8) {
            *identifier = co_msg.info.id;
            *dlc = co_msg.info.length;
            memcpy(data, co_msg.data, *dlc);
        }
    }
    return err;
}

/**
 * Register a callback function for a given ranges of message IDs
 * @param cb_node pointer to callback node
 */
void mcan_task_register_cb(msg_callback_t *cb_node)
{
    can_id_tree = binary_range_insert(can_id_tree, &cb_node->node);
    if(can_id_tree == NULL) {
        /* Programming error - Overlapping ranges perhaps? */
        TraceE1(TrcMsgErr1, "ERORR: msg id high:0x%x low:0x%x",
                cb_node->node.range_high, cb_node->node.range_low,0,0,0,0);
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __builtin_software_breakpoint();
#endif
    }
}

/**
 * Initialize the MCAN0 if present
 */
void mcan_init_MCAN0(void)
{
    MCAN0_MessageRAMConfigSet(Can0MessageRAM);
    MCAN0_TxCallbackRegister(mcan_task_ISR_callback_tx, (uintptr_t)MCAN0_CONTEXT_TX );
    MCAN0_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN0_CONTEXT_RX_FIFO_0, MCAN_MSG_ATTR_RX_FIFO0);
    MCAN0_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN0_CONTEXT_RX_FIFO_1, MCAN_MSG_ATTR_RX_FIFO1 );
    MCAN0_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN0_CONTEXT_RX_BUF, MCAN_MSG_ATTR_RX_BUFFER );
    /* Read number of buffers used for fifo/queue size and shift down into 
     * meaningful bit position */
    UBaseType_t fifo_free_elements = 
            (MCAN0_REGS->MCAN_TXBC & MCAN_TXBC_TFQS_Msk) >> MCAN_TXBC_TFQS_Pos;
    
    xMCAN0Semaphore = xSemaphoreCreateCountingStatic(fifo_free_elements, 
            fifo_free_elements, &xMCAN0SemaphoreBuffer);
}
/**
 * Initialize the MCAN1 if present
 */
void mcan_init_MCAN1(void)
{
    MCAN1_MessageRAMConfigSet(Can1MessageRAM);
    MCAN1_TxCallbackRegister(mcan_task_ISR_callback_tx, (uintptr_t)MCAN1_CONTEXT_TX );
    MCAN1_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN1_CONTEXT_RX_FIFO_0, MCAN_MSG_ATTR_RX_FIFO0);
    MCAN1_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN1_CONTEXT_RX_FIFO_1, MCAN_MSG_ATTR_RX_FIFO1 );
    MCAN1_RxCallbackRegister(mcan_task_ISR_callback_rx,
            (uintptr_t)MCAN1_CONTEXT_RX_BUF, MCAN_MSG_ATTR_RX_BUFFER );
    /* Read number of buffers used for fifo/queue size and shift down into 
     * meaningful bit position */
    UBaseType_t fifo_free_elements = 
            (MCAN1_REGS->MCAN_TXBC & MCAN_TXBC_TFQS_Msk) >> MCAN_TXBC_TFQS_Pos;
    xMCAN1Semaphore = xSemaphoreCreateCountingStatic(fifo_free_elements, 
            fifo_free_elements, &xMCAN1SemaphoreBuffer);
}


/**
 * Clear out any messages in mcan buffers
 */
static void mcan_flush(void)
{
    Mailbox8Type mailbox;
    unsigned int num_entries = mt_rx_fifo_msg_mcan0_get(CAN_RX_FIFO_0, &mailbox);
    /* @FIXME these loops need a limit */
    while(num_entries != 0) {
        num_entries = mt_rx_fifo_msg_mcan0_get(CAN_RX_FIFO_0, &mailbox);
    }
    
    num_entries = mt_rx_fifo_msg_mcan1_get(CAN_RX_FIFO_0, &mailbox);
    while(num_entries != 0) {
        num_entries = mt_rx_fifo_msg_mcan1_get(CAN_RX_FIFO_0, &mailbox);
    }
}


/**
 * Initialize the mcan.  External and client interfaces are selected using
 * the provided pointer.   The external control and client control can be
 * on the same CAN bus or either can be on either bus.
 * @param setup
 */
void mcan_task_init(can_bus_setup_t *setup)
{
    /* @fixme initialize both memories for now, since we have to use different
     *        configurations that use a single or both mcans.  The RX routines
     *        above look for messages on both regardless of whether or not
     *        the interface is used.
     */    
    bool do_mcan0_init = false;
    bool do_mcan1_init = false;
    
    switch(setup->client) {
        case CAN_BUS_1:
            do_mcan1_init = true;
            mcan_tx_client.ptx  = MCAN1_MessageTransmit;
            mcan_tx_client.psem = &xMCAN1Semaphore;
            break;
        case CAN_BUS_0:
        default: /* Internal defaults to zero */
            do_mcan0_init = true;
            mcan_tx_client.ptx  = MCAN0_MessageTransmit;
            mcan_tx_client.psem = &xMCAN0Semaphore;
            break;
    }
    switch(setup->external) {
        case CAN_BUS_0:
            do_mcan0_init = true;
            mcan_tx_extern.ptx  = MCAN0_MessageTransmit;
            mcan_tx_extern.psem = &xMCAN0Semaphore;
            break;
        case CAN_BUS_1:
        default: /* External defaults to one */
            do_mcan1_init = true;
            mcan_tx_extern.ptx  = MCAN1_MessageTransmit;
            mcan_tx_extern.psem = &xMCAN1Semaphore;
            break;
    }
    if(do_mcan0_init) {
        mcan_init_MCAN0();
    }
    if(do_mcan1_init) {
        mcan_init_MCAN1();
    }
    
    mcan_flush();
    
    xTaskCreateStatic(mt_rx_fifo0_task, "Rx FIFO 0 task", rxFifo_TASK_STACK_SIZE,
            NULL, rxFifo_TASK_PRIORITY, rxFifo0Stack, &rxFifo0TaskBuffer);
    xTaskCreateStatic(mt_rx_fifo1_task, "Rx FIFO 1 task", rxFifo_TASK_STACK_SIZE,
            NULL, rxFifo_TASK_PRIORITY, rxFifo1Stack, &rxFifo1TaskBuffer);
    xCoQueue = xQueueCreateStatic( BUFFER_COUNT_CO, QUEUE_ITEM_SIZE_CO,
            ucCoQueueStorageArea, &xCoStaticQueue );    
}


/* *****************************************************************************
 End of File
 */

