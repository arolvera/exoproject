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
/******************************************************************************
* INCLUDES
******************************************************************************/
#include <string.h>                     //  memset

#include "FreeRTOS.h"                   // RTOS defines
#include "task.h"                       // RTOS tasks defines

#include "app_emcy.h"
#include "co_callbacks.h"// callback functions for CAN Open
#include "co_obj.h"                  // can open types
#include "thruster_control.h"
#include "co_project_spec.h"// get external node specification
#include "task_priority.h"

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

/* Sends TIME PDO when seconds are updated if enable */
#define CANOPEN_TIMER_TPDO 0

/* Allocate a global CANopen node object */
CO_NODE tc_node;

/******************************************************************************
* RTOS Definition
******************************************************************************/
#define coClock_TASK_STACK_SIZE ( configMINIMAL_STACK_SIZE * 6 )
StackType_t  coClockStack[coClock_TASK_STACK_SIZE];
StaticTask_t coClockTaskBuffer;

static int send_hb = 0;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

/* timer callback function */
static void co_clock(void *p_arg)
{
    CO_NODE  *node;
    CO_OBJ   *od_sec;
    CO_OBJ   *od_min;
    CO_OBJ   *od_hr;
    uint8_t   second;
    uint8_t   minute;
    uint32_t  hour;

    /* For flexible usage (not needed, but nice to show), we use the argument
     * as reference to the CANopen node object. If no node is given, we ignore
     * the function call by returning immediatelly.
     */
    node = (CO_NODE *)p_arg;
    if (node == 0) {
        return;
    }

    /* Main functionality: when we are in operational mode, we get the current
     * clock values out of the object dictionary, increment the seconds and
     * update all clock values in the object dictionary.
     */
    if (CONmtGetMode(&node->Nmt) == CO_OPERATIONAL) {

        od_sec = CODictFind(&node->Dict, CO_DEV(0x2100, 3));
        od_min = CODictFind(&node->Dict, CO_DEV(0x2100, 2));
        od_hr  = CODictFind(&node->Dict, CO_DEV(0x2100, 1));

        COObjRdValue(od_sec, node, (void *)&second, sizeof(second), 0);
        COObjRdValue(od_min, node, (void *)&minute, sizeof(minute), 0);
        COObjRdValue(od_hr , node, (void *)&hour  , sizeof(hour),   0);

        second++;
        if (second >= 60) {
            second = 0;
            minute++;
        }
        if (minute >= 60) {
            minute = 0;
            hour++;
        }

        COObjWrValue(od_hr , node, (void *)&hour  , sizeof(hour),   0);
        COObjWrValue(od_min, node, (void *)&minute, sizeof(minute), 0);
        COObjWrValue(od_sec, node, (void *)&second, sizeof(second), 0);
    }
}

/******************************************************************************
 *************** Internal NMT Functions       *********************************
 ******************************************************************************
 * Below are the NMT States that we transition to internally.
 *  These end up call the CO Callbacks, so do not get yourself into a
 * infinite loop by calling theses FROM the CO call backs
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

/**
 * Transition to the error state.  The error state could be 'stopped' or just
 * back to 'pre-op'.  See note below.
 * 
 * In the stopped mode, SDO transfers are not allowed.  So it is imperative
 * that the heartbeat is running. 
 * 
 * If the heartbeat has already been set by the user, or its at its default,
 * leave.  If it is off, turn it back on to its default.
 * 
 */
void canopen_task_nmt_error_state(void)
{
    int mode;
/* Define this to re-enable going to Stopped mode on an error.  It was disabled
 * because half-duplex systems have collisions with the heartbeat.  Originally,
 * full-duplex systems would go into the stopped state.  However, that means
 * some of our systems behave differently, which leads to confusion. Therefore,
 * stopped mode was disabled all together.  If that changes, define the
 * stop mode and full-duplex systems will go into the stopped state.
 */
#if STOP_MODE_ENABLED 
    /* If the system is full-duplex we CAN go into the stopped state
     * if not, then we have to go back to pre-operational, because we cannot
     * have the heartbeat on, and SDO transfers are NOT allowed in the
     * stopped state 
     */
    mode = Full_Duplex ? CO_STOP:CO_PREOP;
#else
    mode = CO_PREOP;
#endif
    CONmtSetMode(&tc_node.Nmt, mode);
}
/**
 * Transition to the pre-op state
 */
void canopen_task_nmt_preop(void)
{
    CONmtSetMode(&tc_node.Nmt, CO_PREOP);
}

/**
 * Transition to the operational state and turn the heartbeat back on
 */
void canopen_task_nmt_op(void)
{
    CONmtSetMode(&tc_node.Nmt, CO_OPERATIONAL);
}

/**
 * Force a Heartbeat if the heart beat is turned off.  Some of our configurations
 * are half-duplex, so having the heartbeat on causes collisions.  In that
 * case, make the NMT mode changes work like command-response.  The heartbeat is
 * is off, and when the NMT mode changes, it forces exactly 1 Heartbeat.
 */
void canopen_task_send_hb(void)
{
    if(Heartbeat_Milliseconds == 0) {
        send_hb = 1;
    }
}

void canopen_task_enable_heartbeat(void)
{
    /* Enable the heartbeat whenever we transition states */
    if(Heartbeat_Milliseconds == 0) {
        Heartbeat_Milliseconds = HEARTBEAT_MILLISECONDS;
        CONmtHbProdInit(&tc_node.Nmt);
    }
}
void canopen_task_node_start(void)
{
    /* Start the CANopen node and set it automatically to NMT mode:
     * 'Pre-OPERATIONAL'. */
    CONodeStart(&tc_node);
}

/******************************************************************************
 *************** End Internal NMT Functions ***********************************
 ******************************************************************************/


/* task entry function */
static void canopen_task(void *pvParameters)
{
    /* Initialize your hardware layer and the CANopen stack.
     * Stop execution if an error is detected.
     */
    /* BSPInit(); */


    /* In the background loop we process received CAN frames
     * and executes elapsed action callback functions.
     */
    while (1) {
        CONodeProcess(&tc_node);
        COTmrProcess(&tc_node.Tmr);
        if(send_hb) {
            send_hb = 0;
            CONmtHbProdSend(&tc_node.Nmt);
        }
        task_flags |= TASK_FLAG_CAN_OPEN;
    }
}

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/
/**
 * Create task for CAN Open
 */
void co_task_init(void)
{
        
    COTmrLockCreate();
    
    AppSpec.Dict = CreateDir();
    AppSpec.EmcyCode = EmcyGetTable();
    
    CONodeInit(&tc_node, &AppSpec);
    if (CONodeGetErr(&tc_node) != CO_ERR_NONE) {
        while(1);
    }
#if CANOPEN_TIMER_TPDO
    /* Use CANopen software timer to create a cyclic function
     * call to the callback function 'AppClock()' with a period
     * of 1s (equal: 1000ms).
     */
    uint32_t ticks = COTmrGetTicks(&tc_node.Tmr, 1000, CO_TMR_UNIT_1MS);
    COTmrCreate(&tc_node.Tmr, 0, ticks, co_clock, &tc_node);
#else
    (void)(co_clock);
#endif
    
    xTaskCreateStatic(canopen_task, "CAN Open Task", coClock_TASK_STACK_SIZE,
            NULL, CANOPEN_TASK_PRIO, coClockStack, &coClockTaskBuffer);
}
