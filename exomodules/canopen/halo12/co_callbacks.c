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

#include "co_core.h"
#include "definitions.h"            // Harmony definitions
#include "co_task.h"
#include "trace/trace.h"
#include "thruster_control.h"
#include "client-control/client_service.h"
#include "client-control/power/client_power.h"

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

void COTmrLock  (void);
void COTmrUnlock(void);

OSAL_MUTEX_HANDLE_TYPE co_tmr_mutex;
OSAL_STATIC_MUTEX_BUF_DECLARE(co_tmr_buf);

/******************************************************************************
* MANDATORY CALLBACK FUNCTIONS
******************************************************************************/

void CONodeFatalError(void) 
{
    volatile uint8_t debugExit = 0u;

    /* Place here your fatal error handling.
     * There is most likely a programming error.
     * !! Please don't ignore this errors. !!
     */
    for (;debugExit == 0u;);
}

void COTmrLockCreate(void)
{
    OSAL_MUTEX_Create(&co_tmr_mutex, &co_tmr_buf, "co_tmr");
}

void COTmrLock(void)
{
    /* This function helps to guarantee the consistancy
     * of the internal timer management while interrupted
     * by the used timer interrupt. Most likely you need
     * at this point on of the following mechanisms:
     * - disable the used hardware timer interrupt
     * - get a 'timer-mutex' from your RTOS (ensure to
     *   call COTmrService() in a timer triggered task)
     */
    OSAL_MUTEX_Lock(&co_tmr_mutex, OSAL_WAIT_FOREVER);
}

void COTmrUnlock(void)
{
    /* This function helps to guarantee the consistancy
     * of the internal timer management while interrupted
     * by the used timer interrupt. Most likely you need
     * at this point on of the following mechanisms:
     * - (re)enable the used hardware timer interrupt
     * - release the 'timer-mutex' from your RTOS (ensure
     *   to call COTmrService() in a timer triggered task)
     */
    OSAL_MUTEX_Unlock(&co_tmr_mutex);
}

/******************************************************************************
* OPTIONAL CALLBACK FUNCTIONS
******************************************************************************/

void CONmtModeChange(CO_NMT *nmt, CO_MODE mode)
{
    (void)nmt;
    
    static int first_powerup = 1;
    
    /* This is a global to track the thruster state without tieing EVERYTHING
     * to the CAN Open Stack  */
    Thruster_NMT_State = mode;

    switch(mode){
        case CO_INIT:
            /**
             * We get into this state either through power on event or by command
             * (RESET_NODE and what else?). If power on event, then the clients
             * are being powered on at the same time and will reset their components
             * to default state. If we are commanded to reset, then the clients
             * may be running and we will need to reset them.
             */
            if(first_powerup) {
                first_powerup = 0;
            } else {
                // First send reset command to clients, stop the keeper, then reset self.
                cp_reset_all();
                __NVIC_SystemReset();
            }
            TraceDbg(TrcMsgAlways, "Switched to NMT Init", 0,0,0,0,0,0);        // TrcMsgNmt
            break;
        case CO_PREOP:
            /**
             * We can get here from the CO_INIT or OPERATIONAL states. If coming from
             * init then we shouldn't need to reset the clients and it may be a problem
             * doing so. However, if coming from operational then we do need to reset.
             * We will need to watch out for this to make sure there are no issues.
             */
            client_reset_queue();
            TraceDbg(TrcMsgAlways, "Switched to NMT Pre-OP", 0,0,0,0,0,0);
            break;
        case CO_OPERATIONAL:
            // Clients are already powered due to hardware design so we want to just
            // reset them to get their boot messages. In the future, if hardware is
            // redesigned, we will call client_power_queue()
            client_reset_queue();
            TraceDbg(TrcMsgAlways, "Switched to NMT Operational", 0,0,0,0,0,0);
            break;
        case CO_STOP:
            client_reset_queue();
            /* Enable the heart beat in the stopped state
             * Because if it is off, no SDO transfer can turn it back on */
            canopen_task_enable_heartbeat();
            TraceDbg(TrcMsgAlways, "Switched to NMT Stop", 0,0,0,0,0,0);
            break;
        default:
            break;
    }

    canopen_task_send_hb();
}

void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId)
{
    (void)nmt;
    (void)nodeId;

    /* Optional: place here some code, which is called
     * called when heartbeat consumer is in use and
     * detects an error on monitored node(s).
     */
}

void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE mode)
{
    (void)nmt;
    (void)nodeId;
    (void)mode;

    /* Optional: place here some code, which is called
     * when heartbeat consumer is in use and detects a
     * NMT state change on monitored node(s).
     */
}

int16_t COLssLoad(uint32_t *baudrate, uint8_t *nodeId)
{
    (void)baudrate;
    (void)nodeId;

    /* Optional: place here some code, which is called
     * when LSS client is in use and the CANopen node
     * is initialized.
     */
    return (0u);
}

int16_t COLssStore(uint32_t baudrate, uint8_t nodeId)
{
    (void)baudrate;
    (void)nodeId;

    /* Optional: place here some code, which is called
     * when LSS client is in use and the CANopen node
     * needs to store updated values.
     */
    return (0u);
}

void COIfCanReceive(CO_IF_FRM *frm)
{
    (void)frm;

    /* Optional: place here some code, which is called
     * when you need to handle CAN messages, which are
     * not part of the CANopen protocol.
     */
}

void COPdoTransmit(CO_IF_FRM *frm)
{
    (void)frm;

    /* Optional: place here some code, which is called
     * just before a PDO is transmitted. You may adjust
     * the given CAN frame which is send afterwards.
     */
}

int16_t COPdoReceive(CO_IF_FRM *frm)
{
    (void)frm;

    /* Optional: place here some code, which is called
     * right after receiving a PDO. You may adjust
     * the given CAN frame which is written into the
     * object dictionary afterwards or suppress the
     * write operation.
     */
    return(0u);
}

void COPdoSyncUpdate(CO_RPDO *pdo)
{
    (void)pdo;

    /* Optional: place here some code, which is called
     * right after the object dictionary update due to
     * a synchronized PDO.
     */
}

int16_t COParaDefault(CO_PARA *pg)
{
    (void)pg;

    /* Optional: place here some code, which is called
     * when a parameter group is restored to factory
     * settings.
     */
    return (0u);
}
