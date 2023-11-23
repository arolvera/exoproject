/**
 * @file    limits_object.h
 *
 * @brief   ??? Register the limits object (0x4003) dictionary entries.
 *
 * Why do we have this?
 *   it is not used anywhere, is it named something else?
 *   it is not included in cmake
 *   OD_INDEX_CONTROL_LIMITS is not defined anywhere, not even halo 6 or fcp
 *
 * maybe come back and implement for debug. Will be important when we test thruster.
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

#ifndef LIMITS_OBJECT_H
#define	LIMITS_OBJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app_dict.h"



void LimitsOD(OD_DYN *self);



#ifdef	__cplusplus
}
#endif

#endif	/* LIMITS_OBJECT_H */

