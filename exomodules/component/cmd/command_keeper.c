/**
 * @file    command_keeper.c
 *
 * @brief   Implementation for keeper commands common to both Halo devices.
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

#include "command_keeper.h"
#include "keeper/control_keeper.h"

/* Mutex to protect OD entries */
static OSAL_MUTEX_HANDLE_TYPE mutex_keeper;



uint8_t cmd_keeper_ps_state_get(void)
{
    uint8_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_ps_state_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

int cmd_keeper_ps_state_set(uint8_t on)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    err = ctrl_keeper_ps_state_set(on);
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return err;
}

uint16_t cmd_keeper_state_get(void)
{
    uint8_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_state_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

uint16_t cmd_keeper_state_stat_get(void)
{
    uint8_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_state_stat_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

float_t cmd_keeper_cur_get(void)
{
    float_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_cur_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

uint16_t cmd_keeper_cur_factored_get(void)
{
    uint16_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_cur_factored_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

int cmd_keeper_cur_set(float_t current)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    err = ctrl_keeper_cur_set(current);
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return err;
}

float_t cmd_keeper_volts_get(void)
{
    float_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_volts_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

uint16_t cmd_keeper_volts_factored_get(void)
{
    uint16_t ret = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    ret = ctrl_keeper_volts_factored_get();
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return ret;
}

int cmd_keeper_volts_set(float_t voltage)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    err = ctrl_keeper_volts_set(voltage);
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return err;
}

int cmd_keeper_start(uint32_t timeout)
{
    int err = 0;
    OSAL_MUTEX_Lock(&mutex_keeper, OSAL_WAIT_FOREVER);
    err = ctrl_keeper_start(timeout);
    OSAL_MUTEX_Unlock(&mutex_keeper);
    return err;
}

int cmd_keeper_reinit(void)
{
    return ctrl_keeper_reinit();
}

int cmd_keeper_init(void)
{
    int err = 0;
    static StaticSemaphore_t xMutexBuffer;
    mutex_keeper = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    err = ctrl_keeper_init();
    return err;
}
