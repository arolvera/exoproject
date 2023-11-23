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
#include <stddef.h>

static hp_callback_t interrupt_callback_2;
static hp_callback_t interrupt_callback_3;

// add more cases and interrupt callbacks as needed
// for now everything is assumed to be PORTA, but that may change in the future
void hardware_port_cb_register(hp_callback_t hp_ic, uint8_t port)
{
    switch(port)
    {
        case 2:
            interrupt_callback_2 = hp_ic;
            break;
        case 3:
            interrupt_callback_3 = hp_ic;
            break;
    }
}

void PA2_IRQHandler(void)
{
    if(interrupt_callback_2 != NULL){
        (*interrupt_callback_2)();
    }
}

void PA3_IRQHandler(void)
{
    if(interrupt_callback_3 != NULL){
        (*interrupt_callback_3)();
    }
}