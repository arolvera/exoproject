/* 
 * File:   HardwarePort.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 1:38 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>
#include "HardwarePort.h"

/******************************************************************
 * Setup hardware port pins.  Writing a one sets pin to output.
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
void HardwarePortInit(void)
{
    DDRC = 0x80;          // Set DAC pin to output
    PORTC &= ~(0x80);     // Clear output pin low
    DDRD = 0x01;          // Set LED pin to output
    LED_OFF;
    DDRB = 0x80;          // Set enable pin to output
    BUCK_DISABLE;
}

