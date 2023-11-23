/**
 * @file    magnets_object.h
 *
 * @brief   ??? Register the magnets object (0x2300) and magnets diagnostic (0x3003)
 * dictionary entries.
 *
 * Why is magnet_obj_add_telemetry broken out into a halo12 file?
 * We can considate
 *
 * Anything else to do?
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

#ifndef CO_MAGNET_USER_OBJECT_H
#define CO_MAGNET_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes
#include "user_object_od_indexes.h"


    
/**
 * @brief Register the magnets object (0x2300) and magnets diagnostic (0x3001)
 * dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void MagnetsOD(OD_DYN *self);

void magnet_obj_add_telemetry(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_MAGNET_USER_OBJECT_H */
