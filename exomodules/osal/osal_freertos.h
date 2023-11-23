/*******************************************************************************
  Operating System Abstraction Layer for FreeRTOS

  Company:
    Microchip Technology Inc.

  File Name:
    osal_freertos.h

  Summary:
    OSAL FreeRTOS implementation interface file

  Description:
    Interface file to allow FreeRTOS to be used with the OSAL
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef _OSAL_FREERTOS_H
#define _OSAL_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

/* declare default data type handles. Any RTOS port must define it's own copy of these */
typedef SemaphoreHandle_t              OSAL_SEM_HANDLE_TYPE;
typedef SemaphoreHandle_t              OSAL_MUTEX_HANDLE_TYPE;
typedef StaticSemaphore_t              OSAL_STATIC_MUTEX_BUF;
typedef StaticSemaphore_t              OSAL_STATIC_SEM_BUF;
typedef BaseType_t                     OSAL_CRITSECT_DATA_TYPE;
typedef BaseType_t                     OSAL_BASE_TYPE;
typedef UBaseType_t                    OSAL_UNSIGNED_BASE_TYPE;
typedef TaskFunction_t                 OSAL_TASK_FUNCTION_TYPE;
typedef TaskHandle_t                   OSAL_TASK_HANDLE_TYPE;
typedef StackType_t                    OSAL_STATIC_STACK_BUFFER_TYPE;
typedef StaticTask_t                   OSAL_STATIC_STACK_TYPE;
typedef StaticTask_t * const           OSAL_STATIC_TASK_BUFFER_TYPE;
typedef TickType_t                     OSAL_TICK_TYPE;
typedef QueueHandle_t                  OSAL_QUEUE_HANDLE_TYPE;
typedef StaticQueue_t                  OSAL_STATIC_QUEUE_TYPE;

#define OSAL_WAIT_FOREVER               (uint16_t)0xFFFF
#define OSAL_STATIC_MUTEX_BUF           StaticSemaphore_t

#define OSAL_SEM_DECLARE(semID)         OSAL_SEM_HANDLE_TYPE   semID
#define OSAL_MUTEX_DECLARE(mutexID)     OSAL_MUTEX_HANDLE_TYPE mutexID
#define OSAL_STATIC_MUTEX_BUF_DECLARE(mutexID)     OSAL_STATIC_MUTEX_BUF mutexID
// *****************************************************************************
/* OSAL Result type

  Summary:
    Enumerated type representing the general return value from OSAL functions.

  Description:
    This enum represents possible return types from OSAL functions.

  Remarks:
    These enum values are the possible return values from OSAL functions
    where a standard success/fail type response is required. The majority
    of OSAL functions will return this type with a few exceptions.
*/

typedef enum OSAL_SEM_TYPE
{
  OSAL_SEM_TYPE_BINARY,
  OSAL_SEM_TYPE_COUNTING
} OSAL_SEM_TYPE;

typedef enum OSAL_CRIT_TYPE
{
  OSAL_CRIT_TYPE_LOW,
  OSAL_CRIT_TYPE_HIGH
} OSAL_CRIT_TYPE;

typedef enum OSAL_RESULT
{
  OSAL_RESULT_NOT_IMPLEMENTED = -1,
  OSAL_RESULT_FALSE = 0,
  OSAL_RESULT_TRUE = 1
} OSAL_RESULT;



// *****************************************************************************
// *****************************************************************************
// Section: Section: Interface Routines Group
// *****************************************************************************
// *****************************************************************************
OSAL_RESULT OSAL_SEM_Create(OSAL_SEM_HANDLE_TYPE* semID, OSAL_SEM_TYPE type, uint8_t maxCount, uint8_t initialCount);
OSAL_RESULT OSAL_SEM_Delete(OSAL_SEM_HANDLE_TYPE* semID);
OSAL_RESULT OSAL_SEM_Pend(OSAL_SEM_HANDLE_TYPE* semID, uint16_t waitMS);
OSAL_RESULT OSAL_SEM_Post(OSAL_SEM_HANDLE_TYPE* semID);
OSAL_RESULT OSAL_SEM_PostISR(OSAL_SEM_HANDLE_TYPE* semID);
uint8_t OSAL_SEM_GetCount(OSAL_SEM_HANDLE_TYPE* semID);
OSAL_RESULT OSAL_SEM_CreateStatic(OSAL_SEM_HANDLE_TYPE *semID,
                                  OSAL_SEM_TYPE type,
                                  uint8_t maxCount,
                                  uint8_t initialCount,
                                  OSAL_STATIC_MUTEX_BUF* staticSemBuf,
                                  char *sem_name);

OSAL_CRITSECT_DATA_TYPE OSAL_CRIT_Enter(OSAL_CRIT_TYPE severity);
void  OSAL_CRIT_Leave(OSAL_CRIT_TYPE severity, OSAL_CRITSECT_DATA_TYPE status);

OSAL_RESULT OSAL_MUTEX_Create(OSAL_MUTEX_HANDLE_TYPE *mutexID, void *static_buffer, char *mtx_name);
OSAL_RESULT OSAL_MUTEX_Delete(OSAL_MUTEX_HANDLE_TYPE* mutexID);
OSAL_RESULT OSAL_MUTEX_Lock(OSAL_MUTEX_HANDLE_TYPE* mutexID, unsigned long waitMS);
OSAL_RESULT OSAL_MUTEX_Unlock(OSAL_MUTEX_HANDLE_TYPE* mutexID);

OSAL_TASK_HANDLE_TYPE OSAL_TASK_CreateStatic(OSAL_TASK_FUNCTION_TYPE pxTaskCode,
                                   const char * const pcName,
                                   const uint32_t ulStackDepth,
                                   void * const pvParameters,
                                   OSAL_UNSIGNED_BASE_TYPE uxPriority,
                                   OSAL_STATIC_STACK_BUFFER_TYPE* puxStackBuffer,
                                   OSAL_STATIC_TASK_BUFFER_TYPE pxTaskBuffer );
void OSAL_TASK_Delay(const OSAL_TICK_TYPE xTicksToDelay);

void* OSAL_Malloc(size_t size);
void OSAL_Free(void* pData);

OSAL_RESULT OSAL_Initialize(void);

OSAL_RESULT OSAL_QUEUE_Receive(OSAL_QUEUE_HANDLE_TYPE xQueue,
                               void * const pvBuffer,
                               OSAL_TICK_TYPE xTicksToWait);

OSAL_RESULT OSAL_QUEUE_Send(OSAL_QUEUE_HANDLE_TYPE xQueue,
                            const void * const pvItemToQueue,
                            OSAL_TICK_TYPE xTicksToWait);

OSAL_RESULT OSAL_QUEUE_CreateStatic(OSAL_QUEUE_HANDLE_TYPE *handle,
                                    const OSAL_UNSIGNED_BASE_TYPE uxQueueLength,
                                    const OSAL_UNSIGNED_BASE_TYPE uxItemSize,
                                    uint8_t *pucQueueStorage,
                                    OSAL_STATIC_QUEUE_TYPE *pxStaticQueue,
                                    char *q_name);

void OSAL_TASK_Delete(OSAL_TASK_HANDLE_TYPE xTaskToDelete);

// *****************************************************************************
/* Function: const char* OSAL_Name()

  Summary:
    Obtain the name of the underlying RTOS.

  Description:
    This function returns a const char* to the textual name of the RTOS.
    The name is a NULL terminated string.

  Precondition:
    None

  Parameters:
    None

  Returns:
    const char* -   Name of the underlying RTOS or NULL

  Example:
    <code>
    // get the RTOS name
    const char* sName;

    sName = OSAL_Name();
    sprintf(buff, "RTOS: %s", sName);
    </code>

  Remarks:

 */
static inline __attribute__((always_inline)) const char* OSAL_Name (void)
{
    return "FreeRTOS";
}


// *****************************************************************************
// *****************************************************************************
// Section: Helper Macros
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Macro: OSAL_ASSERT
 */
#define OSAL_ASSERT(test, message)      test


// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
/* These function declarations help map OSAL function calls into specific
   FreeRTOS calls or OSAL translation layer functions
   Each OSAL should fully implement the functions listed in osal.h so only
   deviations from that interface are required here.
 */

#define OSAL_Malloc(size)   pvPortMalloc(size)

#define OSAL_Free(pData)    vPortFree(pData)

#ifdef __cplusplus
}
#endif

#endif // _OSAL_FREERTOS_H

/*******************************************************************************
 End of File
*/
