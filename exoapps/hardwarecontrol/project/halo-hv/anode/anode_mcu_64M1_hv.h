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

#ifndef ANODE_MCU_H
#define	ANODE_MCU_H

#include "mcu_include.h"

// Different errors for the anode
typedef enum
{
    ANODE_NO_ERROR,
    ANODE_UNDER_CURRENT_ERROR,
    ANODE_OVER_CURRENT_ERROR,
    ANODE_UNDER_VOLTAGE_ERROR,
    ANODE_OVER_VOLTAGE_ERROR,
    ANODE_SPARK_TIMEOUT_ERROR
} anode_error_t;

// hardware scaling factors
#define ANODE_COUNTS_PER_AMPERE          160
#define ANODE_COUNTS_PER_VOLT            2.32

// safe operating limits and targets
// @fixme This should probably be lowered to 2.3A
#define ANODE_OVER_CURRENT                  3    // Amperes
#define ANODE_OVER_VOLTAGE                  350  // Volts
#define ANODE_UNDER_VOLTAGE                 110  // Volts
#define ANODE_MAX_OUTPUT_VOLTAGE            325  // Volts
#define ANODE_MIN_OUTPUT_VOLTAGE            125  // Volts

// safe operating limits and targets in counts
#define ANODE_OVER_CURRENT_COUNTS  ANODE_OVER_CURRENT * ANODE_COUNTS_PER_AMPERE
#define ANODE_OVER_VOLTAGE_COUNTS  ANODE_OVER_VOLTAGE * ANODE_COUNTS_PER_VOLT
#define ANODE_UNDER_VOLTAGE_COUNTS ANODE_UNDER_VOLTAGE * ANODE_COUNTS_PER_VOLT
#define ANODE_MAX_V_OUT_COUNTS     ANODE_MAX_OUTPUT_VOLTAGE * ANODE_COUNTS_PER_VOLT
#define ANODE_MIN_V_OUT_COUNTS     ANODE_MIN_OUTPUT_VOLTAGE * ANODE_COUNTS_PER_VOLT

#define DECLARE_ANODE_HEALTH_ENTRY_0(_OUTPUT_VOLTAGE_, _OUTPUT_CURRENT_) { \
    {.data = _OUTPUT_VOLTAGE_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _OUTPUT_CURRENT_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,                .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define DECLARE_ANODE_HEALTH_ENTRY_1(_DAC_, _STATE_, _ERROR_) { \
    {.data = _DAC_,            .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _STATE_,          .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _ERROR_,          .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,                .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#endif	/* ANODE_MCU_H */

