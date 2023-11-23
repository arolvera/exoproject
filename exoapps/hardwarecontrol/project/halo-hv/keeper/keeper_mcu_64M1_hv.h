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

#ifndef KEEPER_MCU_H
#define KEEPER_MCU_H

#include "mcu_include.h"

// Different errors for the keeper
typedef enum
{
    KEEPER_NO_ERROR,
    KEEPER_UNDER_CURRENT_ERROR,
    KEEPER_OVER_CURRENT_ERROR,
    KEEPER_OVER_VOLTAGE_STARTUP_STATE_ERROR,  // before the spark
    KEEPER_OVER_VOLTAGE_ON_STATE_ERROR,       // after the spark
    KEEPER_SPARK_TIMEOUT_ERROR,
} keeper_error_t;

// hardware scaling factors
#define KEEPER_COUNTS_PER_AMPERE          85.00
#define KEEPER_COUNTS_PER_VOLT            1.452

// safe operating limits and targets
#define KEEPER_MAX_OUTPUT_CURRENT            2.3  // Amperes
#define KEEPER_MAX_OUTPUT_VOLTAGE            550  // Volts
#define KEEPER_OVER_VOLTAGE_STARTUP_STATE    600  // Volts
#define KEEPER_OVER_VOLTAGE_ON_STATE         60   // Volts

// safe operating limits and targets in counts
#define KEEPER_MAX_I_OUT_COUNTS             KEEPER_MAX_OUTPUT_CURRENT * KEEPER_COUNTS_PER_AMPERE
#define KEEPER_MAX_V_OUT_COUNTS             KEEPER_MAX_OUTPUT_VOLTAGE * KEEPER_COUNTS_PER_VOLT
#define KEEPER_OVER_VOLTAGE_COUNTS_STARTUP  KEEPER_OVER_VOLTAGE_STARTUP_STATE * KEEPER_COUNTS_PER_VOLT
#define KEEPER_OVER_VOLTAGE_COUNTS_ON       KEEPER_OVER_VOLTAGE_ON_STATE * KEEPER_COUNTS_PER_VOLT

#define DECLARE_KEEPER_HEALTH_ENTRY_0(_OUTPUT_VOLTAGE_, _INPUT_VOLTAGE_, _OUTPUT_CURRENT_) { \
    {.data = _OUTPUT_VOLTAGE_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _INPUT_VOLTAGE_,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _OUTPUT_CURRENT_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,                .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define DECLARE_KEEPER_HEALTH_ENTRY_1(_DAC_, _STATE_, _ERROR_) { \
    {.data = _DAC_,            .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _STATE_,          .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _ERROR_,          .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,                .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#endif	/* KEEPER_MCU_H */

