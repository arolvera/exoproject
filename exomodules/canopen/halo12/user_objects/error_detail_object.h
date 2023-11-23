/**
* @file    error_detail_object.h
*
* @brief   ??? Register the error detail object (0x3108) dictionary entries.
*
* Why do we have this?
*   it is commented out of user_object_config
*   it is not included in cmake
*   the implementation is broken
*   it is not documented in either the internal or external ICD
 *
 * Used to change fault handling strategies. Like disabling or changing in flight for debugging. Never been used in flight but could be? Document this internally.
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

#ifndef CO_ERROR_DETAIL_OBJECT_H
#define CO_ERROR_DETAIL_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * @brief Register the error detail object (0x3108) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void ErrorDetailOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_ERROR_DETAIL_OBJECT_H */

