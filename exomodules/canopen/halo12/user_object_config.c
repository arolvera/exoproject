/**
 * @file:   user_object_config.c
 *
 * @brief:  Implementation for user object definitions.
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

#include "user_object_config.h"
#include "keeper_object.h"
#include "anode_object.h"
#include "sequence_table_update_object.h"
#include "magnets_object.h"
#include "valves_object.h"
#include "update_object.h"
#include "thruster_command_object.h"
#include "firmware_versions_object.h"
#include "hk_hsi_object.h"
#include "throttle_object.h"
#include "mem_corrupter_object.h"
#include "error_detail_object.h"
#include "calib_vals_ucv_object.h"
#include "erase_nor_flash_object.h"

#if __DEBUG
#include "trace/trace.h"
#include "trace_object.h"
#include "health_object.h"
#include "power_control_object.h"
#include "client_control_ucv_object.h"
#include "user_config_var_object.h"
#include "control_condition_object.h"
#include "user_object_od_indexes.h"
#endif  // __DEBUG



typedef void (* USER_OBJECT_INIT_FUNC)( OD_DYN *self );

#define SYS_CONFIG_PLANET           1
#define SYS_CONFIG_TIPPING_POINT    2



/* Arrays of User Objects included in the dictionary -  Try to keep them in OD
 * index order for efficiency, they will be stored in a binary tree, so it is
 * easier to insert them each at the end */
static USER_OBJECT_INIT_FUNC user_object_inits[] = {
    KeeperOD,           // 0x2200
    AnodeOD,            // 0x2200
    MagnetsOD,          // 0x2300
    ValvesOD,           // 0x2500
    HkHsiOD,            // 0x3100
//    ErrorDetailOD,      // 0x3008
    ThrusterCommandOD,  // 0x4000
    ThrottleOD,         // 0x4002
//    SqncCtrlOD,         // 0x4200
    UpdateOD,           // 0x5500
    FirmwareVersionsOD, // 0x5000
//    MemCorrupterOD,      // 0x5005
//    ClientControlUCVOD, // 0x5100
//    CalibValsUCVOD,     // 0x5103
    EraseNorOD,
};
#define OBJ_ARRAY_SIZE (sizeof(user_object_inits)/sizeof(USER_OBJECT_INIT_FUNC))

#if __DEBUG
/* Remember to keep these in index order */
static USER_OBJECT_INIT_FUNC user_debug_inits[] = {
//    CtrlCondOD,        //0x5102
    TraceDebugOD,
    HealthOD,
//    PowerControlOD,
//    UserConfigVarOD,
};

#define DBG_ARRAY_SIZE (sizeof(user_debug_inits)/sizeof(USER_OBJECT_INIT_FUNC))
#endif  // __DEBUG



void user_object_init(OD_DYN *self)
{
    for(uint32_t i = 0; i < OBJ_ARRAY_SIZE; i++) {
        USER_OBJECT_INIT_FUNC func = user_object_inits[i];
        func(self);
    }

#if __DEBUG
    for(uint32_t i = 0; i < DBG_ARRAY_SIZE; i++) {
        USER_OBJECT_INIT_FUNC func = user_debug_inits[i];
        func(self);
    }
#endif  // __DEBUG
}
