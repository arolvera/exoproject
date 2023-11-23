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

#include "app_dict.h"
#include "user_object_od_indexes.h"
#include "user-setting-values/client_control_usv.h"

typedef enum {
    LIMITS_SUBIDX_COUNT                     =  0,
    LIMITS_SUBIDX_KEEPER_OV_LIMIT           =  1,
    LIMITS_SUBIDX_KEEPER_SS_SHUTDOWN_LIMIT  =  2,
    LIMITS_SUBIDX_KEEPER_SS_HIGHWARN_LIMIT  =  3,
    LIMITS_SUBIDX_KEEPER_SS_HIGHCLEAR_LIMIT =  4,
    LIMITS_SUBIDX_KEEPER_SS_LOW_WARN_LIMIT  =  5,
    LIMITS_SUBIDX_KEEPER_SS_LOW_CLEAR_LIMIT =  6,
    LIMITS_SUBIDX_MAGNET_CURRENT_ERROR      =  7,
    LIMITS_SUBIDX_INPUT_POWER_LOW           =  8,
    LIMITS_SUBIDX_INPUT_POWER_HIGH          =  9,
    LIMITS_SUBIDX_EOL                       = 10,
} OD_LIMITS;

void LimitsOD(OD_DYN *self)
{
    safety_check_limits_t *limits = client_limits_safety_limits_get();
    
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_COUNT,
            CO_UNSIGNED8 | CO_OBJ_D__R_), 0, LIMITS_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_OV_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->keeper_limits.critical);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_SS_SHUTDOWN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_shutdown);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_SS_HIGHWARN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_warning);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_SS_HIGHCLEAR_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_clear);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_SS_LOW_WARN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_low_warning);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_KEEPER_SS_LOW_CLEAR_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_low_clear);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_MAGNET_CURRENT_ERROR,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->magnet_current_error);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_INPUT_POWER_LOW,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->vin_range.low);
    ODAdd(self, CO_KEY(OD_INDEX_CONTROL_LIMITS, LIMITS_SUBIDX_INPUT_POWER_HIGH,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->vin_range.high);
}

