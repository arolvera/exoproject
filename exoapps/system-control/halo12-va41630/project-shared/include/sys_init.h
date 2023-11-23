/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SYSTEM_ARMCM4_H
#define SYSTEM_ARMCM4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//Already defined in FreeRtosConfig
#ifndef FREE_RTOS
extern uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock) */
#else
#include "FreeRTOSConfig.h"
#endif
typedef enum {
    SYS_CRASH_TRUE  = 0xDEADBEEF,
    SYS_CRASH_FALSE =  0x00000000
} crash_status_t;
void sys_init(void) __attribute__((weak));
void sys_ebi_init(void);

/**
 * Set or clear crash status to avoid boot loops of doom
 * @param crash_status - SYS_CRASH_TRUE on crash, FALSE on success
 * @note if you're using this, you probably need to back waayyyyy up and check why
 */
void sys_stat_crash_set(crash_status_t crash_status);

void sys_stat_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_ARMCM4_H */
