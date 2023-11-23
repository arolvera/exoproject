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

#include <stdbool.h>
#include <stddef.h>
#include "adc/hal_adc.h"
#include "osal/osal.h"

static OSAL_MUTEX_HANDLE_TYPE adcMtx;
static OSAL_STATIC_MUTEX_BUF_DECLARE(xMutexBuffer);

#define ADC_HISTORY_SIZE 3
typedef struct adc_node {
    uint16_t channel_values[ADC_HISTORY_SIZE]; // ADC value history
    uint8_t current_value_pos;                 // Latest reading position in history array
    uint8_t hw_chan;                           // Hardware channel for ADMUX[4:0]
    uint16_t median_value;                     // Holds median value of current history
    data_ready_func_t data_ready;              // ADC ready callback function
    /* LEAVE THIS AT THE END */
    volatile struct adc_node *next;            // Pointer to next ADC to scan
} ADC_data_t;

static volatile bool all_channels_ready = false;// set after all channels have been read

static volatile struct adc_node *head = NULL;
static volatile struct adc_node *tail = NULL;
static volatile struct adc_node *current = NULL;

#define ADC_MAX_CHANNELS 16
static volatile ADC_data_t ADC_data[ADC_MAX_CHANNELS]; // Setup ADC data structure memory

static bool adc_started = false;

bool adc_data_ready_check(void)
{
    return all_channels_ready;
}

inline void adc_data_ready_reset(void)
{
    all_channels_ready = false;
}

/**
 * Get an ADC value
 * @param data_type instantaneous or median filtered
 * @param channel the channel you want to read
 * @return the requested ADC value
 */
uint16_t adc_get_value(adc_return_t data_type, uint8_t channel)
{
    uint16_t data = 0xFFFF;
    if(channel < ADC_MAX_CHANNELS) {
        volatile ADC_data_t *pADC = &ADC_data[channel];
        switch(data_type) {
            case RETURN_INST:
                data = pADC->channel_values[pADC->current_value_pos];
                break;
            case RETURN_MEDIAN:
                data = pADC->median_value;
                break;
            default:
                break;
        }
    }
    return data;
}

/**
 * Register a new ADC channel
 * @param adc_chan the new channel you want to register
 * @param cb the function you want called when that channel's data is ready
 */
// The order you register the channels is also the scan order, so do the important ones last
void adc_channel_register(uint8_t adc_chan, data_ready_func_t cb)
{
    OSAL_MUTEX_Lock(&adcMtx, (uint32_t)portMAX_DELAY); // only used in rtos

    if(adc_chan < ADC_MAX_CHANNELS) {
        if(tail == NULL) {
            tail = &ADC_data[adc_chan];// List is empty
        } else {
            tail->next = &ADC_data[adc_chan];// Make the old tail point to new tail
            tail = &ADC_data[adc_chan];      // Now make this the new tail
        }
        if(head == NULL) {
            head = tail;
        }

        tail->current_value_pos = 0;
        tail->median_value = 0;
        for(int i = 0; i < ADC_HISTORY_SIZE; i++) {
            tail->channel_values[i] = 0;
        }
        tail->data_ready = cb;
        tail->next = head;
        tail->hw_chan = adc_chan;
    }

    OSAL_MUTEX_Unlock(&adcMtx); // only used in rtos
}

/**
 * Deregister an ADC channel
 * @param adc_chan the channel you want to deregister
 */
void adc_channel_deregister(uint8_t adc_chan)
{
    OSAL_MUTEX_Lock(&adcMtx, 0); // only used in rtos

    if(adc_chan < ADC_MAX_CHANNELS) {
        uint8_t i;
        // if you are deregistering the head, make what the head points to the new head
        if(head == &ADC_data[adc_chan]) {
            head = ADC_data[adc_chan].next;
            tail->next = head;
        }
        // if you are deregistering the tail, find the channel that points to the tail and make it the new tail
        else if(tail == &ADC_data[adc_chan]) {
            i = 0;
            while(ADC_data[i].next != &ADC_data[adc_chan] && i < ADC_MAX_CHANNELS) {
                i++;
            }
            tail = &ADC_data[i];
            ADC_data[i].next = head;
        }
        // otherwise, find the channel that points to the channel you are deregistering,
        // and make it point to the next channel
        else {
            i = 0;
            while(ADC_data[i].next != &ADC_data[adc_chan] && i < ADC_MAX_CHANNELS) {
                i++;
            }
            if(i != ADC_MAX_CHANNELS) {
                ADC_data[i].next = ADC_data[adc_chan].next;
            }
        }
    }

    OSAL_MUTEX_Unlock(&adcMtx); // only used in rtos
}

void adc_start(void)
{
    if(adc_started == false) {
        current = head;
        adc_enable(head->hw_chan);
        adc_started = true;
    }
}

// only to be called by the task runner in rtos implementation
// creates the ADC mutex
void adc_init(void)
{
    OSAL_MUTEX_Create(&adcMtx, &xMutexBuffer, "adc");
}

/**
 * Generic adc interrupt callback function
 * @param value the new adc value
 * @return the next channel to read, or 0xFF if things weren't initialized properly
 */
uint8_t adc_interrupt_callback(uint16_t value)
{
    uint8_t next_channel = 0xFF;

    // check to see if things were initialized properly
    if(current != NULL) {
        volatile ADC_data_t *fresh_data = current;
        fresh_data->current_value_pos++;
        if(fresh_data->current_value_pos == ADC_HISTORY_SIZE) {
            fresh_data->current_value_pos = 0;
        }

        fresh_data->channel_values[fresh_data->current_value_pos] = value;

        /* Highly debated median selection algorithm :) */
        volatile uint16_t *p = fresh_data->channel_values;
        if(p[0] > p[1]) {
            if(p[1] > p[2]) {
                fresh_data->median_value = p[1];
            } else if(p[0] > p[2]) {
                fresh_data->median_value = p[2];
            } else {
                fresh_data->median_value = p[0];
            }
        } else {
            if(p[2] > p[1]) {
                fresh_data->median_value = p[1];
            } else if(p[2] > p[0]) {
                fresh_data->median_value = p[2];
            } else {
                fresh_data->median_value = p[0];
            }
        }

        // run the data ready function if one was given
        if(fresh_data->data_ready != NULL) {
            fresh_data->data_ready(value);
        }

        // Have we read them all?
        if(current == tail) {
            all_channels_ready = true;
        }

        current = fresh_data->next;// advance to the next one

        next_channel = current->hw_chan;
    }
    return next_channel;
}
