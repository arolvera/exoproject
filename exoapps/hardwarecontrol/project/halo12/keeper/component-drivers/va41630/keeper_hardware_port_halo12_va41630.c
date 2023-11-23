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

#include "hardware_port/hal_hardware_port.h"
#include "keeper_event_handlers.h"
#include "keeper_hardware_port.h"
#include "keeper_mcu.h"
#include <device.h>

#define SPARK_DETECT       3 // PORTA[3]

void khp_spark_detect_enable(bool enable)
{
    if(enable){
        VOR_GPIO->BANK[0].IRQ_ENB |= (1 << SPARK_DETECT); // enable interrupts on PORTA[3]
    } else {
        VOR_GPIO->BANK[0].IRQ_ENB &= ~(1 << SPARK_DETECT); // disable interrupts on PORTA[3]
    }
}

static void spark_detected_callback(void)
{
    keeper.common.current_state = keeper_spark_detected_handler();
}

void keeper_hardware_port_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_PORTA;
    NVIC_EnableIRQ(PORTA3_IRQn);                 // enable the PORTA[3] NVIC entry
    NVIC_SetPriority(PORTA3_IRQn, 0);     // set the interrupt priority
    VOR_GPIO->BANK[0].IRQ_EVT |= (1 << SPARK_DETECT); // set PORTA[3] to interrupt on rising edge

    hardware_port_cb_register(spark_detected_callback, SPARK_DETECT);
}
