/**
 * @file    power_control_object.h
 *
 * @brief   ??? Register the power control object (0x5003) dictionary entries.
 *
 * Used to manually power clients. We may want to add it in.
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is not documented in either the internal or external ICD
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

#ifndef CO_POWERC_USER_OBJECT_H
#define CO_POWERC_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes
#include "definitions.h"
#include "health/health.h"// Health data structures includes



/**
 * @brief Register the power control object (0x5003) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void PowerControlOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_POWERC_USER_OBJECT_H */
