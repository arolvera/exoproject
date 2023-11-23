/* 
 * File:   DAC_M64.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 2:37 PM
 */

#ifndef DAC_M64_H
#define	DAC_M64_H

void DAC_init(void);                 // Initialize the DAC hardware
void setDAC(uint16_t DAC_Value);     // Set DAC output  value

#endif	/* DAC_M64_H */

