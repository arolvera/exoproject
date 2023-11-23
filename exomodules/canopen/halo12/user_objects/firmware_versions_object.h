/**
 * @file:   firmware_versions_object.h
 *
 * @brief:  Register the firmware versions object (0x5000) dictionary entries.
 *
 * @copyright   Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef CO_FIRMWARE_VERSION_USER_OBJECT_H
#define CO_FIRMWARE_VERSION_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * @brief Register the firmware versions object (0x5000) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void FirmwareVersionsOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_FIRMWARE_VERSION_USER_OBJECT_H */
