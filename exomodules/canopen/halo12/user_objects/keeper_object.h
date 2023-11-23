/**
 * @file    keeper_object.h
 *
 * @brief   ??? Register the keeper object (0x2100) and keeper diagnostic (0x3001)
 * dictionary entries.
 *
 * We register only 8 of the 9 diagnostic objects published in the external ICD. Why?
 * The 9th appears to be error history and there is a for loop commented out.
 * Delete healt_history_buf and everything associated.
 * REgister the can error object that is missing
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

#ifndef CO_KEEPER_USER_OBJECT_H
#define CO_KEEPER_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes
#include "health/health.h"       // Health data structures includes



/**
 * @brief Register the keeper object (0x2100) and anode diagnostic (0x3001)
 * dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void KeeperOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_KEEPER_USER_OBJECT_H */
