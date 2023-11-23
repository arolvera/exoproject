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

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*ct_callback_t)(void);

typedef enum {
    TIMER_MICRO,
    TIMER_MILLI,
} timer_units_t;

typedef struct {
    uint8_t channel;
    uint32_t rst_value;
    uint8_t interrupt_priority;
    timer_units_t timer_units;
    ct_callback_t interrupt_callback;
} timer_init_t;

typedef struct {
    uint8_t channel;
    uint32_t cnt_value;
    timer_units_t timer_units;
} timer_set_t;

void timer_init(timer_init_t *tmr_init);
void timer_start(uint8_t channel, bool enable);
void timer_enable_interrupts(uint8_t channel, bool enable);
void timer_set(timer_set_t *timer_set);
uint32_t timer_value_get(uint8_t channel);
void timer_cb_register(ct_callback_t ct_ic, uint8_t channel);
void timer_deinit(timer_init_t *tmr_init);

typedef enum {
    TIMER_ZERO,
    TIMER_ONE,
    TIMER_TWO,
    TIMER_THREE,
    TIMER_FOUR,
    TIMER_FIVE,
    TIMER_SIX,
    TIMER_SEVEN,
    TIMER_EIGHT,
    TIMER_NINE,
    TIMER_TEN,
    TIMER_ELEVEN,
    TIMER_TWELVE,
    TIMER_THIRTEEN,
    TIMER_FOURTEEN,
    TIMER_FIFTEEN,
    TIMER_SIXTEEN,
    TIMER_SEVENTEEN,
    TIMER_EIGHTEEN,
    TIMER_NINETEEN,
    TIMER_TWENTY,
    TIMER_TWENTY_ONE,
    TIMER_TWENTY_TWO,
    TIMER_TWENTY_THREE,
    NUMBER_OF_TIMERS
} timer_channels_t;

#endif /* HAL_TIMER_H */
