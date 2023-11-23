/**
 * @file    valves_object.h
 *
 * @brief   ??? Provide a table of emergency message types to the canopen stack.
 *
 * Why is EmcyResetTable defined here? It is implemented and used in the third-party
 * canopen stack. It wa stest suite which is not compiled
 *
 * Why do we define EmcyGetTable here? it is defined in the canopen stack.
 *
 * Why is EmcyAddCode defined here? It is implemented and used in the third-party
 * canopen stack.
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef APP_EMCY_H
#define APP_EMCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_core.h"



/**
 * @brief CLEAR EMCY TABLE
 * @details Clear the configuration table which mapps the supported emergency
 *          codes of the test environment to the error register code.
 */
void EmcyResetTable(void);

/**
 * @brief GET EMCY TABLE
 * @details Retrieve the pointer to the emergency map table.
 * @return Pointer to the emergency map table
 */
CO_EMCY_TBL *EmcyGetTable(void);

/**
 * @brief ADD EMCY CODE TO EMCY TABLE
 * @details Add a new mapping from an emergency error code to the
 *          corresponding error register information.
 * @param code The emergency error code
 *          (e.g. 'CO_EMCY_CODE_VOL_ERR + x' for a specific voltage error)
 * @param reg The error register information for this emergency error
 *          (e.g. 'CO_EMCY_REG_VOLTAGE' maps to the voltage bit in register)
 * @return Index of new entry in table, or EMCY_CODE_MAX if table is full.
 */
uint32_t EmcyAddCode(int16_t code, uint8_t reg);



#ifdef __cplusplus
}
#endif

#endif  // APP_EMCY_H
