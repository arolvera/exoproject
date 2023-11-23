/**
 * @file    control_condition_object.h
 *
 * @brief   ??? Register the control condition object (0x5102) dictionary entries.
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is included in cmake
 *   the implementation appears incomplete
 *   it is not documented in either the internal or external ICD
 *   halo12 will not require conditioning
 *
 *  delete it
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

#ifndef CO_CONTROL_CONDITION_USER_OBJECT_H
#define CO_CONTROL_CONDITION_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * Add neccessary entries into the Dynamic Object Dictionary
 * @param self pointer to dynamic object dictionary 
 */
void CtrlCondOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif  // CO_CONTROL_CONDITION_USER_OBJECT_H