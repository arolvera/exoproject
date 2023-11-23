/* 
 * File:   ADC_M64.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 1:53 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>
#include "ADC_M64.h"

#define ADC_ACTIVE_CHANNELS      4    // Number of channels to convert
#define ADC_MAX_CHANNELS         16
#define ADC_ARRAY_SIZE           3    // Size of ADC sample array, 3 for filter

// Scan order of ADCs
#define ADC_CHANNEL_I_OUT        0
#define ADC_CHANNEL_V_OUT        1
#define ADC_CHANNEL_I_IN         2
#define ADC_CHANNEL_V_IN         3

// ADC hardware channels
#define HARDWARE_CHANNEL_I_OUT   2 
#define HARDWARE_CHANNEL_V_OUT   6
#define HARDWARE_CHANNEL_I_IN    10
#define HARDWARE_CHANNEL_V_IN    5

typedef struct {
    int16_t channel_values[ADC_ARRAY_SIZE]; // ADC value history
    uint8_t current_value_pos;              // Latest reading position in history array
    uint8_t hw_chan;                        // Hardware channel for ADMUX[4:0]
    int16_t median_value;                   // Holds median value of current history
} ADC0_t;

static volatile ADC0_t ADC0[ADC_ACTIVE_CHANNELS]; // Setup ADC data structure memory
static volatile bool ADC_dataReadyFlag;           // This flag is set after all channels have been read
static volatile uint8_t CurrentChannel;           // Current ADC channel

/******************************************************************
 * Function to return an ADC value in raw or median form.
 *
 * @Parameters: Data type to return and pointer to ADC results
 * @return   return ADC result or median
 * 
 * Revision
 *  - NEW
 *****************************************************************/
static inline uint16_t GetAdcValue(adc_return_t DataType, volatile ADC0_t *pAdc)
{
    uint16_t ret = 0;
    switch (DataType) {
        case RETURN_INST:
            ret = pAdc->channel_values[ pAdc->current_value_pos ];
            break;
        case RETURN_MEDIAN:
            ret = pAdc->median_value;
            break;
        default:
            ret = -1; // wrong input
    }
    return ret;
}

/******************************************************************
 * Return output current. Either instantaneous or median
 *
 * Parameters: Data type
 * @return   return output current
 * 
 * Revision
 *  - NEW
 *****************************************************************/
uint16_t GetOutputCurrent(adc_return_t DataType)
{
    return GetAdcValue(DataType, &ADC0[ADC_CHANNEL_I_OUT]);
}

/******************************************************************
 * Return output voltage. Either instantaneous or median
 *
 * Parameters: data type
 * @return   return output voltage
 * 
 * Revision
 *  - NEW
 *****************************************************************/
uint16_t GetOutputVoltage(adc_return_t DataType)
{
    return GetAdcValue(DataType, &ADC0[ADC_CHANNEL_V_OUT]);
}

/******************************************************************
 * Return input current. Either instantaneous or median
 *
 * Parameters: Data type
 * @return   return output current
 * 
 * Revision
 *  - NEW
 *****************************************************************/
uint16_t GetInputCurrent(adc_return_t DataType)
{
    return GetAdcValue(DataType, &ADC0[ADC_CHANNEL_I_IN]);
}

/******************************************************************
 * Return input voltage. Either instantaneous or median
 *
 * Parameters: data type
 * @return   return output voltage
 * 
 * Revision
 *  - NEW
 *****************************************************************/
uint16_t GetInputVoltage(adc_return_t DataType)
{
    return GetAdcValue(DataType, &ADC0[ADC_CHANNEL_V_IN]);
}

/******************************************************************
 * Return data ADC ready flag
 *
 * Parameters: none
 * @return   return ADC data ready flag
 * 
 * Revision
 *  - NEW
 *****************************************************************/
uint8_t ADC_dataReadyFlagGet(void)
{
    return ADC_dataReadyFlag ? 1 : 0; //Return data ready flag
}

/******************************************************************
 * Reset the ADC data ready flag
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
inline void ADC_dataReadyFlagReset(void)
{
    ADC_dataReadyFlag = false; //Clear data ready flag
}

/******************************************************************
 * Initialize the ADC module hardware and start first conversion
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
void ADC0_init(void)
{
    CurrentChannel = 0; //Setup current and active channels
    
    for(int i = 0; i < ADC_ACTIVE_CHANNELS; i++) {
        ADC0[i].current_value_pos = 0;
        ADC0[i].median_value = 0;
        for(int j = 0; j < ADC_ARRAY_SIZE; j++) {
            ADC0[i].channel_values[j] = 0;
        }
    }
    
    ADC0[ADC_CHANNEL_I_OUT].hw_chan = HARDWARE_CHANNEL_I_OUT;
    ADC0[ADC_CHANNEL_V_OUT].hw_chan = HARDWARE_CHANNEL_V_OUT;
    ADC0[ADC_CHANNEL_I_IN].hw_chan  = HARDWARE_CHANNEL_I_IN;
    ADC0[ADC_CHANNEL_V_IN].hw_chan  = HARDWARE_CHANNEL_V_IN;

    // Take ADC out of power down mode
    PRR &= ~0b00000001;
 
    //ADMUX: [7:6] = REFS1; [5] = ADLAR; [4:0] = MUX
    ADMUX = 0b11000000;                     //Set internal Vref on, disable left adjust
    ADMUX |= ADC0[CurrentChannel].hw_chan;  //start with first ADC channel in array

    //ADCSRA: [7] = ADEN; [6] = ADSC; [5] = ADATE; [4] = ADIF; [3] = ADIE; [2:0] = ADPS
    ADCSRA = 0b10001110;    //Enable ADC, disable auto trigger, enable interrupt, clock prescaler /64
    
    //This setup makes the ADC sample time per channel 41.25uS
    //ADCSRB: [7] ADHSM; [6] = ISRCEN; [5] = AREFEN; [3:0] = ADTS
    ADCSRB = 0b00000000; //enable high speed mode, disable current source, don't connect ADC ref to external pin, free run mode

    ADCSRA |= 0b01000000; //Start ADC conversion
}

/******************************************************************
 * ISR - ADC interrupt service routine.  Read active channel and 
 * updates function to next value
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
ISR(ADC_vect)
{
    volatile ADC0_t *pAdc = &ADC0[CurrentChannel];
    pAdc->current_value_pos++;
    if (pAdc->current_value_pos == ADC_ARRAY_SIZE) {
      // Rollover
        pAdc->current_value_pos = 0;
    }
     //Get conversion result
    pAdc->channel_values[pAdc->current_value_pos] = (ADCL | (ADCH << 8));
    
    /* Highly debated Median Selection algorithm :) */
    volatile int16_t *p = pAdc->channel_values;
    if (p[0] > p[1]) {
        if (p[1] > p[2]) {
            pAdc->median_value = p[1];
        } else if (p[0] > p[2]) {
            pAdc->median_value = p[2];
        } else {
            pAdc->median_value = p[0];
        }
    } else {
        if (p[2] > p[1]) {
            pAdc->median_value = p[1];
        } else if (p[2] > p[0]) {
            pAdc->median_value = p[2];
        } else {
            pAdc->median_value = p[0];
        }
    }
    CurrentChannel++; //Move to next sample index
    if (CurrentChannel >= ADC_ACTIVE_CHANNELS)  //Check for end of all channel samples
    {
        CurrentChannel = 0;                     //Reset channels index to start
        ADC_dataReadyFlag = true;               //Set end of all samples flag
    }
                                                //Increment current channel pointer
    pAdc = &ADC0[CurrentChannel];
    ADMUX &= ~0x1f;                             //Clear channel MUX channel[4:0]
    ADMUX |= (pAdc->hw_chan & 0x1f);            //Configure new MUX channel[4:0]
    ADCSRA |= 0b01000000;                       //Start conversion
}
