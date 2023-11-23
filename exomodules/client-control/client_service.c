/***
 * Gives access to external modules to send commands to client control.
 * Includes wrapper callback functions for commands
 */
#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_SERVICE

#include "client_service.h"
#include "FreeRTOS.h"
#include "client_control.h"
//#include "conditioning/control_condition.h"
#include "power/client_power.h"
#include "queue.h"
#include "trace/trace.h"
#include "conditioning/control_condition.h"

static QueueHandle_t *xServiceQueuePtr;

void client_service_init(QueueHandle_t *service_queue)
{
    xServiceQueuePtr = service_queue;
}

/**
 * Added a function to the client service queue.  The function will be executed
 * when the client_service() function is called, which will de-queue any 
 * outstanding tasks and execute them.
 * @param service pointer to the client service info 
 */
BaseType_t client_service_queue(client_service_t *service)
{
    BaseType_t xStatus = xQueueSend(*xServiceQueuePtr, service, 0);
    if(xStatus != true) {
        TraceE3(TrcMsgErr3, "Too many outstanding tasks", 0,0,0,0,0,0);
    }
    return xStatus;
}

/**********************************************************************************
 * Client Power Control
 *********************************************************************************/
/**
 * Wrapper to power up function can be queue'd for app task to handle
 * 
 * @param boolean for power state
 * @return error if param is NULL
 */
static int client_power_on_wrapper(void* param)
{
    client_power_state_set(true);
    return 0;
}

/**
 * Wrapper that can be queue'd for app task to handle
 * 
 * @param boolean for power state
 * @return error if param is NULL
 */
static int client_power_off_wrapper(void* param)
{
    client_power_state_set(false);
    return 0;
}

void client_power_queue(bool power_on)
{
    client_service_t service;
    service.cb = power_on ? client_power_on_wrapper : client_power_off_wrapper;
    service.params = NULL;
    client_service_queue(&service);
}

/**********************************************************************************
 * Client Reset Control
 *********************************************************************************/
/**
 * Wrapper that can be queue'd for app task to handle
 *
 * @param boolean for power state
 * @return error if param is NULL
 */
static int client_reset_wrapper(void* param)
{
    client_reset();
    return 0;
}

void client_reset_queue(void)
{
    client_service_t service;
    service.cb = client_reset_wrapper;
    service.params = NULL;
    client_service_queue(&service);
}

/**
 * Wrapper to tie the service callback function prototype to the shutdown
 * prototype
 * @param parm unused
 * @return 0
 */
int ctrl_sequence_shutdown(void *parm)
{
    ctrl_sequence_abort();
    ctrl_condition_abort();
    return ctrl_sequence_error_shutdown();
}

/**
 * Queues the shutdown to be serviced later
 * 
 * In this context 'shutdown' means turning off all the controls and NOT
 * powering them off. 
 * 
 */
void client_shutdown(void)
{
    client_service_t service;
    service.cb = ctrl_sequence_shutdown;
    service.params = NULL;
    client_service_queue(&service);
}