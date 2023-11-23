/**
 * @file    update_object.h
 *
 * @brief   Register the update object (0x5500) dictionary entries.
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

#ifndef CO_UPDATE_USER_OBJECT_H
#define CO_UPDATE_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * Add neccessary entries into the Dynamic Object Dictionary
 * @param self pointer to dynamic object dictionary 
 */
void UpdateOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_UPDATE_USER_OBJECT_H */

