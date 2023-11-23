/**
 * @file    hk_hsi_object.h
 *
 * @brief   ??? Register the health diagnostic object (0x3100) dictionary entries.
 * Dumps the entire telemetry for all components under one command.
 *
 * Perhaps rename this?
 *
 * Seems sparse. Is the implementation complete?
 * Why is this named hk_hsi?
 * hk = housekeeping board for Halo 6. For halo 12 it is ECPK.
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

#ifndef CO_HK_HSI_USER_OBJECT_H
#define CO_HK_HSI_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * @brief Register the health diagnostic object (0x3100) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void HkHsiOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_ANODE_USER_OBJECT_H */
