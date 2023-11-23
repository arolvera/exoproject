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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define DECLARE_GLOBALS
#include "ext_decl_define.h"
#include "trace.h"

#ifdef TRACE_UART
#include "serial/hal_serial.h"
#endif

#if defined(__x86_64__)
#include <unistd.h>
#include <pthread.h>
#include <string.h>
const char *fmt_str = "%03u.%06u:%s\n";
#else
const char *fmt_str = "%03lu.%06lu:%s";
#endif

#if _TRACE_ENABLE

static OSAL_STATIC_MUTEX_BUF_DECLARE(xMutexBuffer);

static int traceFd = 0;
// Future task implementation -- Below is example/suedo code for implementing
// a task that will write the trace ring buffer out to some TBD NVM

/***************************************************************************
*                                       traceShowN
***************************************************************************/
uint32_t traceShowN(unsigned int n)
{
    uint32_t num;
    TraceEntry t;
    char format[256], prt[256];

    OSAL_MUTEX_Lock(&traceMtx, portMAX_DELAY);
    for(num = 0; (num < n) && (traceTl != traceHd); num++) {
        t = traceBuf[traceTl];
        traceTl = (traceTl + 1) & (TraceSize - 1);
        OSAL_MUTEX_Unlock(&traceMtx);
        sprintf(format, fmt_str,
                (t.tag % 1000000 / 100000), t.tag % 100000, t.format_str);
        sprintf(prt, format,
                t.data[0], t.data[1], t.data[2], t.data[3], t.data[4], t.data[5]);
#if defined(__x86_64__)
        write(traceFd, prt, strlen(prt));
#elif defined(TRACE_UART)
        uart_write_raw(traceFd, (uint8_t *)prt, strlen(prt));
#endif

        OSAL_MUTEX_Lock(&traceMtx, portMAX_DELAY);
    }
    OSAL_MUTEX_Unlock(&traceMtx);
    return num;
}

/***************************************************************************
*                                   tTrace
* Trace task. Display entries in the trace buffer.
***************************************************************************/
void *tTrace(void *p)
{
    (void)p;
    while(1) {
        if(traceFd > 0) { (void)traceShowN(10); }
#if defined(__x86_64__)
        usleep(10000);
#endif
    }
}

#define FORMAT_MAX 128
/**
 * Get a trace message.  Which end is the head or tail (newest or oldest)
 * The tail will be advanced, the head will be decremented.  In other words
 * it takes the one you got out of the trace buffer.  (more accuratly, its still
 * there, it will just be overwritten at the appropriate time).
 * @param buf string buffer
 * @param max_size maximum string size (including the NULL)
 * @param which_end 0 = head, not zero is the tail
 * @return number of characters printed in buffer
 */
int trcGet(char *buf, int max_size, int which_end)
{
    int ret = 0;
    bool isEmpty = false;

    char format[FORMAT_MAX];
    TraceEntry t;

    OSAL_MUTEX_Lock(&traceMtx, 0);
    if(traceHd == traceTl) {
        isEmpty = true;
    }else {
        if(which_end) {
            t = traceBuf[traceTl];
            traceTl = (traceTl + 1) & (TraceSize - 1);
        }else {
            traceHd = (traceHd - 1) & (TraceSize - 1);
            t = traceBuf[traceHd];
        }
    }
    OSAL_MUTEX_Unlock(&traceMtx);

    if(!isEmpty) {
        ret = snprintf(format, FORMAT_MAX, "%d.%04d:%s:%d:%s",
                       (int)((t.tag / 10000) % 100), (int)(t.tag % 10000), t.func, t.line, t.format_str);
        if(ret >= FORMAT_MAX || ret <= 0) {
            ret = 0;  /* Error - return 'nothing in buffer' */
        }else {
            format[FORMAT_MAX - 1] = 0;
            ret = snprintf(buf, max_size, format,
                           t.data[0], t.data[1], t.data[2], t.data[3], t.data[4], t.data[5]);
            if(ret >= max_size || ret <= 0) {
                ret = 0;  /* Error - return 'nothing in buffer' */
            }else {
                buf[max_size - 1] = 0;
            }
        }
    }
    return ret;
}


#if FREE_RTOS && TRACE_UART
#include "task_priority.h"
#define MAX_TRACE_MSG_SIZE      128
/* Interval in between trace checks */
#define TRACE_EMPTY_WAIT_MS     250
#define trace_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 3 )
static StackType_t  trace_task_stack[trace_TASK_STACK_SIZE];
static StaticTask_t trace_TaskBuffer;

static void trcTask(void *pvParameters)
{
    (void)pvParameters;
    int size;
    char trace_message[MAX_TRACE_MSG_SIZE] = {0};
    
    /* Initialize xNextWakeTime - this only needs to be done once. */
    TickType_t xNextWakeTime = xTaskGetTickCount();
    
    while(1) {
        size = trcGet(trace_message, MAX_TRACE_MSG_SIZE, 1);
        if(size > 0) {
            printf("%s\n\r", trace_message);
        } else {
            vTaskDelayUntil(&xNextWakeTime, (TRACE_EMPTY_WAIT_MS/portTICK_PERIOD_MS));
        }
    }
}
#endif

void trcInit(void)
{
#if defined(FREE_RTOS)
    OSAL_MUTEX_Create(&traceMtx, &xMutexBuffer, "trace");
#endif

#ifdef TRACE_UART
#if defined(FREE_RTOS)
    xTaskCreateStatic(trcTask, "Trace Task",
                      trace_TASK_STACK_SIZE, NULL, TRACE_TASK_PRIO,
                      trace_task_stack, &trace_TaskBuffer);
#endif
    uart_init_t uart0_init = {
        .uart_if_id = UART_IF_ID_0,
        .baud = UART_BAUD_115200,
        .rx_irq_enable = 0,
        .tx_irq_enable = 0,
    };
    traceFd = uart_init(&uart0_init); /* stdout */
#elif defined(__x86_64__)
    traceFd = 1; /* stdout */
    pthread_t thread_id;
    /* initialized with default attributes */
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread_id, &tattr, &tTrace, 0);
#endif
}
#else
void trcInit(void) {}
#endif

int trcUartHandleGet(void)
{
    return traceFd;
}
