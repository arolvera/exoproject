/**
 * @file    user_config_var_object.h
 *
 * @brief   ??? Register the user config variable object (0x5102) dictionary entries.
 *
 * Are we storing user configs in NVM?
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is not included in cmake
 *   it has compile errors
 *   it is not documented in eith internal or external ICD
 *   what kinds of config variables did we envision?
 *
 * Associated with ucv. Keep it? Not likely
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

#ifndef USER_CONFIG_VAR_OBJECT_H
#define	USER_CONFIG_VAR_OBJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



void UserConfigVarOD(OD_DYN *self);



#ifdef	__cplusplus
}
#endif

#endif	/* USER_CONFIG_VAR_OBJECT_H */

