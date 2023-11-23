/**
 * @file    sequence_table_update_object.h
 *
 * @brief   ??? Register the sequence table update object (0x4200) dictionary entries.
 *
 * Are we storing sequence tables in NVM? Or are sequence changes volatile? They are volatile.
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is not included in cmake
 *   it has compile errors
 *   it only registers 9 of the 9 documented objects
 *   who would do this and when?
 *
 * This was super duper useful according to Jeremy
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

#ifndef CO_ANODESTARTUP_USER_OBJECT_H
#define CO_ANODESTARTUP_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * @brief Register the sequence table update object (0x4200) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void SqncCtrlOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_ANODESTARTUP_USER_OBJECT_H */
