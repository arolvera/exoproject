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
#include "magnet_event_handlers.h"
#include "magnet_hardware_port.h"
#include "magnet_mcu.h"
#include "device.h"

#define OVER_CURRENT       2 // PORTA[2]

static void over_current_callback(void)
{
    magnet.common.current_state = magnet_error_handler();
}

void magnet_hardware_port_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_PORTA;
    NVIC_EnableIRQ(PORTA2_IRQn);                 // enable the PORTA[2] NVIC entry
    NVIC_SetPriority(PORTA2_IRQn, 0);     // set the interrupt priority
    VOR_GPIO->BANK[0].IRQ_EVT |= (1 << OVER_CURRENT); // set PORTA[2] to interrupt on rising edge

    hardware_port_cb_register(over_current_callback, OVER_CURRENT);
    VOR_GPIO->BANK[0].IRQ_ENB |= (1 << OVER_CURRENT); // enable interrupts on PORTA[2]
}
