/* 
 * File:   buck_mcu.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 3:07 PM
 */

#ifndef BUCK_MCU_H
#define	BUCK_MCU_H

#include "mcu_include.h"

#define DECLARE_BUCK_HEALTH_ENTRY_0(_I_OUT_, _V_OUT_, _I_IN_, _V_IN_) { \
    {.data = _I_OUT_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _V_OUT_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _I_IN_,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _V_IN_,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,       .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define DECLARE_BUCK_HEALTH_ENTRY_1(_DAC_, _STATE_, _ERROR_CODE_, _ERROR_ADC_) { \
    {.data = _DAC_,        .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _STATE_,      .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _ERROR_CODE_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _ERROR_ADC_,  .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = 0,            .size = HEALTH_ENTRY_SIZE_EOL}, \
}

#define BUCK_HEALTH_ENTRIES 3
typedef health_table_entry_t *buck_health_array[BUCK_HEALTH_ENTRIES];

// Different errors for the 28V buck
typedef enum
{
    NO_ERROR_BUCK,
    OUTPUT_CURRENT_ERROR_BUCK,
    OUTPUT_VOLTAGE_ERROR_BUCK,
    INPUT_CURRENT_ERROR_BUCK,
    INPUT_VOLTAGE_ERROR_BUCK,
    DAC_RAILED_ERROR_BUCK,
    BAD_COMMAND_ERROR_BUCK
} buck_error_t;

#define CAN_OPEN_NODE_ID_BUCK            0x006
#define RESPONSE_PARAMETERS_CAN_ID_BUCK (RESPONSE_PARAMETERS_CAN_ID_BASE | CAN_OPEN_NODE_ID_BUCK)
#define COMMAND_PARAMETERS_CAN_ID_BUCK  (COMMAND_PARAMETERS_CAN_ID_BASE  | CAN_OPEN_NODE_ID_BUCK)
#define BROADCAST_STATE_CAN_ID_BUCK     (BROADCAST_STATE_CAN_ID_BASE     | CAN_OPEN_NODE_ID_BUCK)
#define HEALTH_CAN_ID_BUCK              (HEALTH_CAN_ID_BASE              | CAN_OPEN_NODE_ID_BUCK)
#define BROADCAST_VARIABLE_CAN_ID_BUCK  (BROADCAST_VARIABLE_ID_BASE      | CAN_OPEN_NODE_ID_BUCK)

#define BUCK_MAX_MSG_SIZE 8

// hardware scaling factors
#define BUCK_OUTPUT_COUNTS_PER_AMPERE   79.9
#define BUCK_OUTPUT_COUNTS_PER_VOLT     18.16
#define BUCK_INPUT_COUNTS_PER_AMPERE    79.9
#define BUCK_INPUT_COUNTS_PER_VOLT      6.68
#define BUCK_ZERO_CURRENT_OFFSET        100    // Counts

#endif	/* BUCK_MCU_H */

