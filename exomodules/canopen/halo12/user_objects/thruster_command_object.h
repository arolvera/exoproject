/**
 * @file    thruster_command_object.h
 *
 * @brief   ??? Register the thruster command object (0x4000) dictionary entries.
 *
 * We define subindexes in the header but do it in the c file for every other object.
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

#ifndef CO_THRUSTER_STATE_USER_OBJECT_H
#define CO_THRUSTER_STATE_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"           // For adding Oject Dictionary Entires
#include "co_core.h"            // CAN Open core includes



/**
 * @brief   Enumeration of thruster command sub-indexes. Order must match the order in the Object Dictionary.
 */
typedef enum {
    THRUSTER_COMMAND_SUBIDX_COUNT           = 0,
    THRUSTER_COMMAND_SUBIDX_READY_MODE      = 1, /* Keeper On for Halo 6, not supported for Halo 12 */
    THRUSTER_COMMAND_SUBIDX_STEADY_STATE    = 2, /* Anode On for Halo 6, ignition with first setpoint for Halo 12  */
    THRUSTER_COMMAND_SUBIDX_SHTDN           = 3, /* Shutdown the thruster */
    THRUSTER_COMMAND_SUBIDX_THRUST          = 4, /* Set throttle set point in throttle table */
    THRUSTER_COMMAND_SUBIDX_STAT            = 5, /* Current Status of the thruster */
    THRUSTER_COMMAND_SUBIDX_CONDITION       = 6, /* Run the conditioning sequence(s) */
    THRUSTER_COMMAND_SUBIDX_TEST_BIT        = 7, /* Run the bit test */
    THRUSTER_COMMAND_SUBIDX_START           = 8, /* One button thruster startup */
    THRUSTER_COMMAND_SUBIDX_EOL             = 9, /* End of List */
} od_thruster_command_subidx_t;

/**
 * @brief   Enumeration of thruster command.
 */
typedef enum {
    THRUSTER_COMMAND_OFF,
    THRUSTER_COMMAND_ON,
} thruster_command_t;



/**
 * @brief   Add supported CANOpen objects for thruster control into the Dynamic Object Dictionary.
 *
 * @param   self pointer to dynamic object dictionary
 */
void ThrusterCommandOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_THRUSTER_STATE_USER_OBJECT_H */
