/**
 * @file    health_object.h
 *
 * @brief   ??? Register the health object (0x5002) dictionary entries.
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

#ifndef HEALTH_OBJECT_H
#define	HEALTH_OBJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app_dict.h"



/**
 * @brief Register the health object (0x5002) dictionary entries.
 * @param self pointer to dynamic object dictionary
 */
void HealthOD(OD_DYN *self);



#ifdef	__cplusplus
}
#endif

#endif	/* HEALTH_OBJECT_H */

