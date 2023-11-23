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

#ifndef MAGNET_MCU_H
#define	MAGNET_MCU_H

#include "mcu_include.h"

// Different errors for the magnet
typedef enum
{
    MAGNET_NO_ERROR,
    MAGNET_INNER_CURRENT_ERROR,
    MAGNET_OUTER_CURRENT_ERROR,
    MAGNET_INNER_VOLTAGE_ERROR,
    MAGNET_OUTER_VOLTAGE_ERROR,
    MAGNET_POWER_GOOD_TIMEOUT_ERROR,
    MAGNET_TEMPERATURE_ERROR
} magnet_error_t;

// hardware scaling factors
#define MAGNET_COUNTS_PER_AMPERE          250
#define MAGNET_COUNTS_PER_VOLT            81.0
#define MAGNET_COUNTS_PER_DEGREE_CELCIUS  7.19

// safe operating limits and targets
#define MAGNET_MAX_OUTPUT_CURRENT         3.3  // Amperes
#define MAGNET_MAX_OUTPUT_VOLTAGE         12   // Volts
#define MAGNET_MAX_TEMPERATURE            125  // Degrees Celcius
#define MAGNET_POWER_GOOD_RANGE           0.1 // Amperes

// safe operating limits and targets in counts
#define MAGNET_MAX_I_OUT_COUNTS         MAGNET_MAX_OUTPUT_CURRENT * MAGNET_COUNTS_PER_AMPERE
#define MAGNET_MAX_V_OUT_COUNTS         MAGNET_MAX_OUTPUT_VOLTAGE * MAGNET_COUNTS_PER_VOLT
#define MAGNET_MAX_TEMP_COUNTS          MAGNET_MAX_TEMPERATURE * MAGNET_COUNTS_PER_DEGREE_CELCIUS
#define MAGNET_POWER_GOOD_RANGE_COUNTS  MAGNET_POWER_GOOD_RANGE * MAGNET_COUNTS_PER_AMPERE

#define DECLARE_MAGNET_HEALTH_ENTRY_0(_I_INNER_, _V_INNER_, _I_OUTER_, _V_OUTER_) { \
    {.data = _I_INNER_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _V_INNER_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _I_OUTER_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _V_OUTER_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,         .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define DECLARE_MAGNET_HEALTH_ENTRY_1(_PWM_INNER_, _PWM_OUTER_, _TEMPERATURE_, _STATE_) { \
    {.data = _PWM_INNER_,   .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _PWM_OUTER_,   .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _STATE_,       .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _TEMPERATURE_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,             .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#endif	/* MAGNET_MCU_H */

