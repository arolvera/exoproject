/* 
 * File:   COMP_M64.c
 * Author: fnorwood
 *
 * Created on March 11, 2022, 11:21 AM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <stdbool.h>
#include "BuckControl.h"
#include "eventHandlers.h"

void COMP_init(void)
{
    //AC3CON = 0b11110001;
    AC3CON |= (1 << AC3EN);  // Enable comparator 3
    AC3CON &= ~(1 << AC3IE); // Disable interrupt
    AC3CON |= (1 << AC3IS1); // Interrupt on rising edge
    AC3CON |= (1 << AC3IS0);
    AC3CON &= ~(1 << AC3M2); // negative pin = Vref/3.20 = 0.8V = 4.5A output
    AC3CON &= ~(1 << AC3M1);
    AC3CON |= (1 << AC3M0);
}

ISR(ANACOMP3_vect)
{
    Control.ErrorADC = 0xADDE;    // ErrorADC code for comparator errors
    Control.ErrorCode = OUTPUT_CURRENT_ERROR_BUCK;
    Control.CurrentState = errorHandler();
}