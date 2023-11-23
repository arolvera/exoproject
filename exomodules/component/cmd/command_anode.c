/**
* @file    command_anode.c
*
* @brief   Implementation for anode commands common to both Halo devices.
*
* @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited
* proprietary and confidential.  Any unauthorized use, duplication, transmission,
* distribution, or disclosure of this software is expressly forbidden.
*
* This Copyright notice may not be removed or modified without prior written
* consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
* software without notice.
*/

#include <math.h>
#include "control_anode.h"
#include "definitions.h"

/* Mutex to protect OD entries */
OSAL_MUTEX_HANDLE_TYPE mutex_anode;



uint8_t cmd_anode_ps_state_get(void)
{
    uint8_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    ret = ctrl_anode_ps_state_get();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return ret;
}

int cmd_anode_ps_state_set(uint8_t on)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    err = ctrl_anode_ps_state_set(on);
    OSAL_MUTEX_Unlock(&mutex_anode);
    return err;
}

float_t cmd_anode_cur_get(void)
{
    float_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    ret = ctrl_anode_cur_get();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return ret;
}

uint16_t cmd_anode_cur_factored_get(void)
{
    uint16_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    ret = ctrl_anode_cur_factored_get();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return ret;
}

int cmd_anode_cur_set(float_t current)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    err = ctrl_anode_cur_set(current);
    OSAL_MUTEX_Unlock(&mutex_anode);
    return err;
}

float_t cmd_anode_volts_get(void)
{
    float_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    ret = ctrl_anode_volts_get();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return ret;
}

uint16_t cmd_anode_volts_factored_get(void)
{
    uint16_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    ret = ctrl_anode_volts_factored_get();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return ret;
}

int cmd_anode_volts_set(float_t voltage)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    err = ctrl_anode_volts_set(voltage);
    OSAL_MUTEX_Unlock(&mutex_anode);
    return err;
}

int cmd_anode_monitor_stability(float_t target_current)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    err = ctrl_anode_monitor_stability(target_current);
    OSAL_MUTEX_Unlock(&mutex_anode);
    return err;
}

int cmd_anode_reinit(void)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_anode, OSAL_WAIT_FOREVER);
    err = ctrl_anode_reinit();
    OSAL_MUTEX_Unlock(&mutex_anode);
    return err;
}

int cmd_anode_init(void)
{
    static StaticSemaphore_t xMutexBuffer;
    mutex_anode = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    return ctrl_anode_init();
}