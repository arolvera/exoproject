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

#include "anode_event_handlers.h"
#include "anode_hardware_port.h"
#include "anode_mcu.h"
#include "hardware_port/hal_hardware_port.h"
#include <device.h>

#define SPARK_DETECT       3  // PORTA[3]
#define CONTROL_ENABLE     4  // PORTG[4]
#define INPUT_POWER        4  // PORTA[4]

void ahp_input_power_enable(bool enable)
{
    if(enable){
        VOR_GPIO->BANK[0].SETOUT |= (1 << INPUT_POWER);
    } else {
        VOR_GPIO->BANK[0].CLROUT |= (1 << INPUT_POWER);
    }
}

void ahp_spark_detect_enable(bool enable)
{
    if(enable){
        VOR_GPIO->BANK[0].IRQ_ENB |= (1 << SPARK_DETECT); // enable interrupts on PORTA[3]
    } else {
        VOR_GPIO->BANK[0].IRQ_ENB &= ~(1 << SPARK_DETECT); // disable interrupts on PORTA[3]
    }
}

static void spark_detected_callback(void)
{
    anode.common.current_state = anode_spark_detected_handler();
}

void anode_hardware_port_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_PORTA | CLK_ENABLE_PORTG;
    NVIC_EnableIRQ(PORTA3_IRQn);                 // enable the PORTA[3] NVIC entry
    NVIC_SetPriority(PORTA3_IRQn, 0);     // set the interrupt priority
    VOR_GPIO->BANK[0].IRQ_EVT |= (0 << SPARK_DETECT); // set PORTA[3] to interrupt on falling edge

    VOR_GPIO->BANK[6].DIR |= (1 << CONTROL_ENABLE);   // set PORTG[4] to output
    VOR_GPIO->BANK[0].DIR |= (1 << INPUT_POWER);      // set PORTA[4] to output
    ahp_input_power_enable(false);
    VOR_GPIO->BANK[6].SETOUT |= (1 << CONTROL_ENABLE);

    hardware_port_cb_register(spark_detected_callback, SPARK_DETECT);
}
