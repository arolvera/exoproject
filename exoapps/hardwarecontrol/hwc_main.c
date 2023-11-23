/*
           Copyright (C) 2022 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file via any medium is strictly prohibited.
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this 
 software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#include "adc/hal_adc.h"
#include "component_service.h"
#include "osal/osal.h"
#include "hwc_main.h"
#include "watchdog/hal_watchdog.h"
#include "component_callback.h"
#include "sys_init.h"

#define WDT_TIME 7.5

/* com_id for single component build is set in the project config file */
#ifdef SINGLE_COMPONENT_BUILD
#ifdef EXORUN_SINGLE_COMPONENT_ANODE
#define COMM_ID COMM_ID_ANODE
#elif defined(EXORUN_SINGLE_COMPONENT_KEEPER)
#define COMM_ID COMM_ID_KEEPER
#elif defined(EXORUN_SINGLE_COMPONENT_MAGNET)
#define COMM_ID COMM_ID_MAGNET_O
#elif defined(EXORUN_SINGLE_COMPONENT_VALVE)
#define COMM_ID COMM_ID_VALVE
#endif
#endif  // SINGLE_COMPONENT_BUILD

void hwc_init(void)
{
    static bool is_first_init = true;
    if(is_first_init) {
        adc_init();
#ifndef SINGLE_COMPONENT_BUILD
        init_hsi_cb();
#endif
        is_first_init = false;
    }
}

void hwc_main(void *comm_id)
{
    uint8_t ops_id = commid_2_opsid(*((uint8_t *)comm_id));
    cs_init(ops_id);

    watchdog_enable(WDT_TIME);

    while(1) {
        cs_service(ops_id);
        watchdog_reset(); // pet the dog
    }
}

#if SINGLE_COMPONENT_BUILD
int main(void)
{
    sys_init();
    static int id;
    id = (int)COMM_ID;  // If this is undefined, check the project config to ensure a valid option is enabled
#if FREE_RTOS
#define app_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
    static StackType_t  app_task_stack[app_TASK_STACK_SIZE * 2];
    static StaticTask_t app_TaskBuffer;

    xTaskCreateStatic((TaskFunction_t) hwc_main,
                      "APP_Tasks",
                      app_TASK_STACK_SIZE,
                      &id,
                      tskIDLE_PRIORITY,
                      app_task_stack,
                      &app_TaskBuffer);
    vTaskStartScheduler();
#else
    hwc_main(&id);
#endif  //FREE_RTOS
    return 1;
}
#endif  //SINGLE_COMPONENT_BUILD
