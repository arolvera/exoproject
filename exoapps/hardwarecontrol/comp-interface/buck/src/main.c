/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/
/* 
 * File:   main.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 12:04 PM
 */

#include "ADC_M64.h"
#include "BuckControl.h"
#include "COMP_M64.h"
#include "DAC_M64.h"
#include "HardwarePort.h"
#include "TIMER_M64.h"
#include "common_initialize.h"
#include "commsHandler_M64.h"
#include "component_communication.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <stddef.h>

int main(void) 
{

    cli();    // Disable interrupts
    // The real buck doesn't have an external clock, so the 
    // following line should be commented out.
    //MCU_CLOCK_PRESCALER_CONFIG;
    HardwarePortInit();
    ADC0_init();
    DAC_init();
    Control_init();
    COMP_init();
    Timer_init();
    
    common_initialize(CAN_ID_BUCK, buck_sync);
    common_communication_scaling_report(BUCK_OUTPUT_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT);
    common_communication_scaling_report(BUCK_OUTPUT_COUNTS_PER_VOLT, COUNTS_PER_VOLT_OUTPUT);
    common_communication_scaling_report(BUCK_INPUT_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_INPUT);
    common_communication_scaling_report(BUCK_INPUT_COUNTS_PER_VOLT, COUNTS_PER_VOLT_INPUT);
    
    wdt_enable(WDTO_2S);
    sei();    // Enable interrupts
    
    while(1)
    {
        CANcommHandler();
        BuckControlHandler();
        wdt_reset();
    }
}

