/**
 * @file    anode_object.h
 *
 * @brief   ??? Register the anode object (0x2200) and anode diagnostic (0x3002)
 * dictionary entries.
 *
 * Only 6 of the 10 published commands in the internal ICD are registered. Why?
 * Haven't needed the nissing 4.
 *
 * We register 13 diagnostic objects but only publish 11 in the external ICD. Why?
 * Document the other 2.
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

#ifndef CO_ANODE_USER_OBJECT_H
#define CO_ANODE_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes
#include "health/health.h"       // Health data structures includes



/**
 * @brief Register the anode object (0x2200) and anode diagnostic (0x3002)
 * dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void AnodeOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_ANODE_USER_OBJECT_H */
