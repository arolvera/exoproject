/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/
/**
 * health_object.c
 *
 * @Company Exoterra
 * @File health_object.c
 * @Summary  User object for health control.
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */
#include "health_object.h"
#include "thruster_control.h"
#include "user_object_od_indexes.h"

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    HSI_SUBIDX_COUNT = 0,
    HSI_TICK_SET,
    HSI_TICK_ENABLE,
    HSI_SUBIDX_EOL,
} OD_HSI;


void HealthOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_HSI, HSI_SUBIDX_COUNT,
            CO_UNSIGNED8  | CO_OBJ_D__R_), 0, HSI_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_HSI, HSI_TICK_SET,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&Health_Tick_Millisecs);
    ODAdd(self, CO_KEY(OD_INDEX_HSI, HSI_TICK_ENABLE,
            CO_UNSIGNED8  | CO_OBJ____RW), 0, (uintptr_t)&Health_Tick_Enabled);
}
