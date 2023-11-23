#include "driver_can_x86.h"
#include "osal/osal.h"
#include "msg-handler/msg_handler.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>

#define PORT 8080

#define CAN_RX_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

static OSAL_QUEUE_HANDLE_TYPE can_rx_queue_handle;

static OSAL_MUTEX_HANDLE_TYPE rx_mtx = NULL;
static OSAL_STATIC_MUTEX_BUF rx_mtx_buf;

static OSAL_MUTEX_HANDLE_TYPE tx_mtx = NULL;
static OSAL_STATIC_MUTEX_BUF tx_mtx_buf;

static int socket_fd;

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
        OSAL_MUTEX_Lock(&tx_mtx, OSAL_WAIT_FOREVER);
        int stat = send(socket_fd , msg , sizeof(message_t) , 0);
        OSAL_MUTEX_Unlock(&tx_mtx);
        if (stat < 0) {
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
        OSAL_MUTEX_Lock(&rx_mtx, OSAL_WAIT_FOREVER);
        message_t msg = {0};
        int stat = recv(socket_fd, &msg , sizeof(msg) , MSG_WAITALL );
        OSAL_MUTEX_Unlock(&rx_mtx);

        if (stat > 0) {
            printf("RECEIVED HERE TOO!\n");

            printf("id = %x, data = %x, %x, %x, %x, %x, %x\n", msg.id, msg.data[0], msg.data[1], msg.data[2], msg.data[3], msg.data[4], msg.data[5]);

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

#if defined (SYSTEM_CONTROL_SOCKET_SERVER)
    printf("system control server init.  Waiting for clients...\n");
    int opt = 1;

    int server_fd, valread;
    int new_socket = -1;
    struct sockaddr_in address;

    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET,
               SO_REUSEADDR | SO_REUSEPORT, &opt,
               sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address,
         sizeof(address));

    listen(server_fd, 3);

    while(new_socket == -1) {
        new_socket = accept(server_fd, (struct sockaddr *) &address,
                            (socklen_t *) &addrlen);
    }

    socket_fd = new_socket;
#endif



#if defined (HARDWARE_CONTROL_SOCKET_CLIENT)
    printf("system control client init.\n");
    struct sockaddr_in server;

    //Create socket
    socket_fd = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_fd == -1)
    {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8080 );

    //Connect to remote server
    if (connect(socket_fd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
#endif
    printf("serial init\n");

    int err = 0;
    int handle = 0;

    /* Init shared queue */
    can_rx_queue_handle = *(can_init->rx_q_handle);

    message_t msg = {0};
    msg.id = 0x123;
    msg.dlc = 8;
    msg.data[0] = 1;
    msg.data[1] = 2;
    msg.data[2] = 3;
    msg.data[3] = 4;
    msg.data[4] = 5;
    msg.data[5] = 6;
    msg.data[6] = 7;
    msg.data[7] = 8;

    can_send(socket_fd, &msg, 0xffff);


    OSAL_TASK_HANDLE_TYPE task_handle = OSAL_TASK_CreateStatic(can_rx_task,
                                                               "msg_handler",
                                                               CAN_RX_TASK_STACK_SIZE,
                                                               0,
                                                               tskIDLE_PRIORITY,
                                                               can_rx_task_stack,
                                                               &rx_TaskBuffer);



    return err == -1 ? err : handle;
}