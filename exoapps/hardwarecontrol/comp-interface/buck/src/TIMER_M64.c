/* 
 * File:   TIMER_M64.h
 * Author: fnorwood
 *
 * Created on March 14, 2022, 4:00 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>
#include "TIMER_M64.h"


void Timer_init(void) 
{
    TCCR1B |= (1 << CS12);
    TCCR1B |= (1 << CS10);
}

// Sets a flag every 500us
bool timerFlagGet(void)
{
    return TCNT1 >= 4;
}

void timerFlagReset(void)
{
    TCNT1 = 0;
}