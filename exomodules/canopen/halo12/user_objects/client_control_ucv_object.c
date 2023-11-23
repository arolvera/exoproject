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

#include "client_control_ucv_object.h"
#include "client_control/client_control_ucv.h"
#include "halo12-vorago/canopen/user_object_od_indexes.h"

typedef enum {
    CLIENT_CONTROL_UCV_SUBIDX_COUNT         =  0,
    SERIAL_UCV_BAUD                         =  1,
    CLIENT_CONTROL_UCV_LOCKOUT_TIME         =  2,
    CLIENT_CONTROL_UCV_HSI_MISSES           =  3,
    LIMITS_SUBIDX_KEEPER_OV_LIMIT           =  4,
    LIMITS_SUBIDX_KEEPER_SS_SHUTDOWN_LIMIT  =  5,
    LIMITS_SUBIDX_KEEPER_SS_HIGHWARN_LIMIT  =  6,
    LIMITS_SUBIDX_KEEPER_SS_HIGHCLEAR_LIMIT =  7,
    LIMITS_SUBIDX_KEEPER_SS_LOW_WARN_LIMIT  =  8,
    LIMITS_SUBIDX_KEEPER_SS_LOW_CLEAR_LIMIT =  9,
    LIMITS_SUBIDX_MAGNET_CURRENT_ERROR      =  10,
    LIMITS_SUBIDX_INPUT_POWER_LOW           =  11,
    LIMITS_SUBIDX_INPUT_POWER_HIGH          =  12,
    CLIENT_CONTROL_UCV_SUBIDX_EOL                      
} OD_CLIENT_CONTROL_UCV;

void ClientControlUCVOD(OD_DYN *self)
{
    cc_ucv_t *cc_ucv = cc_ucv_get(0);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, CLIENT_CONTROL_UCV_SUBIDX_COUNT,
            CO_UNSIGNED8 | CO_OBJ_D__R_), 0, CLIENT_CONTROL_UCV_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, SERIAL_UCV_BAUD,
            CO_UNSIGNED8 | CO_OBJ____RW), 0, (uintptr_t)&cc_ucv->serial_baud);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, CLIENT_CONTROL_UCV_LOCKOUT_TIME,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&cc_ucv->lockout_time);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, CLIENT_CONTROL_UCV_HSI_MISSES,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&cc_ucv->mci_hci_misses_allowed);

    safety_check_limits_t *limits = client_limits_safety_limits_get();
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_OV_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->keeper_limits.critical);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_SS_SHUTDOWN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_shutdown);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_SS_HIGHWARN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_warning);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_SS_HIGHCLEAR_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_high_clear);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_SS_LOW_WARN_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_low_warning);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_KEEPER_SS_LOW_CLEAR_LIMIT,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->ss_power_low_clear);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_MAGNET_CURRENT_ERROR,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->magnet_current_error);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_INPUT_POWER_LOW,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->vin_range.low);
    ODAdd(self, CO_KEY(OD_INDEX_CLIENT_CONTROL_UCV, LIMITS_SUBIDX_INPUT_POWER_HIGH,
            CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&limits->vin_range.high);
}

