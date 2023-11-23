/**
 * @file    command_magnets.c
 *
 * @brief   Implementation for magnet commands.
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

#include "magnet/control_magnets.h"

static OSAL_MUTEX_HANDLE_TYPE mutex_magnet;



uint8_t cmd_magnet_state_get(void)
{
    uint8_t ret;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    ret = ctrl_magnet_ps_state_get();
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return ret;
}

int cmd_magnet_state_set(uint8_t state)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    err = ctrl_magnet_ps_state_set(state);
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return err;
}

int cmd_magnets_version_get(uint32_t** version)
{
    return ctrl_magnets_version_get(version);
}

float cmd_magnet_current_get()
{
    float ret;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    ret = ctrl_magnet_current_get();
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return ret;
}

uint16_t cmd_magnet_current_factored_get()
{
    uint16_t ret;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    ret = ctrl_magnet_current_factored_get();
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return ret;
}

int cmd_magnet_current_set(float current)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    err = ctrl_magnet_current_set(current);
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return err;
}

int cmd_magnet_current_check(void)
{
    int err;
    OSAL_MUTEX_Lock(&mutex_magnet, OSAL_WAIT_FOREVER);
    err = ctrl_magnet_current_check();
    OSAL_MUTEX_Unlock(&mutex_magnet);
    return err;
}

int cmd_magnet_init(void)
{
    int err;
    static StaticSemaphore_t xMutexBuffer;
    mutex_magnet = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    err = ctrl_magnet_init();
    return err;
}
