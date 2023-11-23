#include "driver_can_x86.h"
#include "osal/osal.h"
#include "PCANBasic.h"
#include "msg-handler/msg_handler.h"
#include <stdio.h>

#define CAN_RX_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

static OSAL_QUEUE_HANDLE_TYPE can_rx_queue_handle;

static OSAL_MUTEX_HANDLE_TYPE rx_mtx = NULL;
static OSAL_STATIC_MUTEX_BUF rx_mtx_buf;

static OSAL_MUTEX_HANDLE_TYPE tx_mtx = NULL;
static OSAL_STATIC_MUTEX_BUF tx_mtx_buf;

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
    (void)handle;
    (void)timeout;
    int err = 0;
    if(msg == NULL){
        err = __LINE__;
    } else {
        TPCANMsg m = {.ID = msg->id, .LEN = msg->dlc, .MSGTYPE = PCAN_MESSAGE_STANDARD};
        memcpy(m.DATA, msg->data, msg->dlc);
        OSAL_MUTEX_Lock(&tx_mtx, OSAL_WAIT_FOREVER);
        TPCANStatus stat = CAN_Write(PCAN_USBBUS1, &m);
        OSAL_MUTEX_Unlock(&tx_mtx);
        if (stat != PCAN_ERROR_OK) {
            err = __LINE__;
        }
    }
    return err;
}


/**
 * Poll CAN interface for received messages
 *
 * @param pv params passed to task on startup
 *
 * @return void
 */
static void can_rx_task(void* pv)
{
    (void)pv;

    while(1) {
        TPCANMsg m = {0};
        TPCANTimestamp TimestampBuffer;
        OSAL_MUTEX_Lock(&rx_mtx, OSAL_WAIT_FOREVER);
        TPCANStatus stat = CAN_Read(PCAN_USBBUS1, &m, &TimestampBuffer);
        OSAL_MUTEX_Unlock(&rx_mtx);

        if (stat == PCAN_ERROR_OK) {
            message_t msg = {0};
            msg.id = m.ID;
            msg.dlc = m.LEN;
            memcpy(msg.data, m.DATA, m.LEN);

            OSAL_QUEUE_Send(can_rx_queue_handle, &msg, portMAX_DELAY);
        }
    }
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
    static StackType_t can_rx_task_stack[CAN_RX_TASK_STACK_SIZE];
    static StaticTask_t rx_TaskBuffer;

    OSAL_MUTEX_Create(&rx_mtx, &rx_mtx_buf, "rx_mtx");
    OSAL_MUTEX_Create(&tx_mtx, &tx_mtx_buf, "tx_mst");

    int err = 0;
    int handle = 0;

    /* Init shared queue */
    can_rx_queue_handle = *(can_init->rx_q_handle);

    TPCANStatus stat = CAN_Initialize(PCAN_USBBUS1, PCAN_BAUD_1M, 0,0,0);

    if (stat != PCAN_ERROR_OK) {
        err = -1;
    } else {
        OSAL_TASK_HANDLE_TYPE task_handle = OSAL_TASK_CreateStatic(can_rx_task,
                               "msg_handler",
                               CAN_RX_TASK_STACK_SIZE,
                               0,
                               tskIDLE_PRIORITY,
                               can_rx_task_stack,
                               &rx_TaskBuffer);
        if(task_handle == NULL){
            err = -1;
        }
    }

    return err == -1 ? err : handle;
}