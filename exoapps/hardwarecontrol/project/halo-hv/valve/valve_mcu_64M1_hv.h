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

#ifndef VALVE_MCU_H
#define VALVE_MCU_H

#include "mcu_include.h"

// Different errors for the valves
// No valve specific errors yet, but maybe there will be some in the future
typedef enum
{
    VALVE_NO_ERROR,
} valve_error_t;

// hardware scaling factors
#define VALVE_3000_PSI_SENSOR_SCALE       0.320
#define VALVE_40_PSI_SENSOR_SCALE         19.22
#define VALVE_VOLTAGE_SCALE               49.58   // Rev B and after
#define VALVE_THERMISTOR_BETA             3419    // property of NTC thermistors
#define VALVE_THERMISTOR_R_NOUGHT         1000    // resistance at 25C in ohms

// safe operating limits and targets
#define VALVE_MAX_PRESSURE                50   // PSI
#define VALVE_MAX_HIGH_FLOW               1000 // permil (percent times ten)

// safe operating limits and targets in counts
#define VALVE_MAX_PRESSURE_COUNTS         VALVE_MAX_PRESSURE * VALVE_40_PSI_SENSOR_SCALE

#define DECLARE_VALVE_HEALTH_ENTRY_0(__ANODE_V__, __CATH_HF_V__, __CATH_LF_V__, __TEMP__) { \
    {.data = __ANODE_V__,   .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __CATH_HF_V__, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __CATH_LF_V__, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __TEMP__,      .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0, .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define DECLARE_VALVE_HEALTH_ENTRY_1(__TANK_PRES__, __CATH_PRES__, __ANODE_PRES__, __REG_PRES__) { \
    {.data = __TANK_PRES__,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __CATH_PRES__,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __ANODE_PRES__, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = __REG_PRES__,   .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0, .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#endif	/* VALVE_MCU_H */

