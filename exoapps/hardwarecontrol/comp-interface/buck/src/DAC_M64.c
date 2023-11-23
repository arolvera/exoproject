/* 
 * File:   DAC_M64.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 2:37 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>
#include "DAC_M64.h"

/******************************************************************
 * Setup DAC module
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
void DAC_init(void)
{
	//DACON: [7] DAATE; [6] DATS2; [5] DATS1;[4] DATS0; [3] ; [2] DALA; [1] DAOE; [0] DAEN
	DACON = 0b00000111;        //[7] = 0 (Auto trigger), [6-4] = 000 (unused in auto trigger), [2] = 1 (use left adjust for auto trigger) [1] = 1 (output enable), [0] = 1  (enable DAC)
	setDAC(0);
}

/******************************************************************
 * Set DAC output hardware to value
 *
 * @param UPDATE_DAC
 * Returns: none
 * 
 * Revision
 *  - NEW
 *  - Added mask in case extra bits are set to clamp at 10 bits.
 *****************************************************************/
inline void setDAC(uint16_t DAC_Value)
{
    uint16_t DacVal;
    DacVal = DAC_Value & 0x03FF;     // Mask number to 10 bits max
	DACL = (uint8_t)(DacVal << 6);   // Value is setup as left aligned
	DACH = (uint8_t)(DacVal >> 2);   // to allow auto trigger
}