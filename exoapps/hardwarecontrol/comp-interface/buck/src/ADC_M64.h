/* 
 * File:   ADC_M64.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 1:53 PM
 */

#ifndef ADC_M64_H
#define	ADC_M64_H

typedef enum {
    RETURN_INST,    // Return instantaneous sample value
    RETURN_MEDIAN,  // Return sample data array median
} adc_return_t;

// Define Interface Functions
void ADC0_init(void);                               // Initialize ADC module
uint8_t ADC_dataReadyFlagGet(void);                 // Returns data ready flag
void ADC_dataReadyFlagReset(void);                  // Clears data ready flag
uint16_t GetOutputCurrent(adc_return_t DataType);   // Return output current
uint16_t GetOutputVoltage(adc_return_t DataType);   // Return output voltage
uint16_t GetInputCurrent(adc_return_t DataType);    // Return input current
uint16_t GetInputVoltage(adc_return_t DataType);    // Return input voltage

#endif	/* ADC_M64_H */

