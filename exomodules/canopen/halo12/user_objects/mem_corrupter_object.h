/**
 * @file    memory_corrupter_object.h
 *
 * @brief   ??? Register the memory corrupter object (0x5005) dictionary entries.
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is not included in cmake
 *   the implementation is broken
 *   it tries to register 14 objects but only 8 are documented in the ICD
 *
 * This was created to force memory corruption during test to see if ICM did its job.
 * We no longer need this.
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

#ifndef CO_MEM_CORRUPTER_USER_OBJECT_H
#define CO_MEM_CORRUPTER_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes
#include "definitions.h"



/**
 * Add neccessary entries into the Dynamic Object Dictionary
 * @param self pointer to dynamic object dictionary 
 */
void MemCorrupterOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_MEM_CORRUPTER_USER_OBJECT_H */
