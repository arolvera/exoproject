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

#include "device.h"

#define WDT_SECOND  10000000

void watchdog_enable(float timout_seconds)
{
    uint32_t wdt_load = WDT_SECOND * timout_seconds;
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_WDOG; // enable the watchdog clock
    VOR_SYSCONFIG->PERIPHERAL_RESET &= ~(1 << SYSCONFIG_PERIPHERAL_RESET_WDOG_Pos); // reset the watchdog
    __NOP();
    __NOP();
    VOR_SYSCONFIG->PERIPHERAL_RESET |= (1 << SYSCONFIG_PERIPHERAL_RESET_WDOG_Pos);
    VOR_WATCH_DOG->WDOGLOAD = wdt_load;                                   //Watchdog  reset time
    VOR_WATCH_DOG->WDOGCONTROL |= (1 << WATCH_DOG_WDOGCONTROL_INTEN_Pos); // enable watchdog interrupts
    VOR_WATCH_DOG->WDOGCONTROL |= (1 << WATCH_DOG_WDOGCONTROL_RESEN_Pos); // set the watchdog to reset the processor
}

void watchdog_reset(void)
{
    VOR_WATCH_DOG->WDOGINTCLR = 1;
}