/**
 * @file:   user_object_od_indexes.h
 *
 * @brief:  ??? Master list of all defined user objects for Halo. Not all are
 * actually used.
 *
 * Are there any current not used that should be? Or ones that are used but
 * could be removed?
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

#ifndef USER_OBJECT_OD_INDEXES
#define	USER_OBJECT_OD_INDEXES
typedef enum
{
    OD_INDEX_ANODE                  = 0x2200,
    OD_INDEX_ANODE_DIAG             = 0x3002,
    OD_INDEX_ANODE_ERR_HISTORY      = 0x3012,
    OD_INDEX_CLIENT_CONTROL_UCV     = 0x5100,
    OD_INDEX_CALIB_VALS_UCV         = 0x5103,
    OD_INDEX_CONDITION_STAT         = 0x4001,
    OD_INDEX_CONDITION_STAT_CLEAR   = 0x5401,
    OD_INDEX_CONDITION_LIMIT        = 0x5101,
    OD_INDEX_FAULT_REACTION_TYPE    = 0x2830,
    OD_INDEX_FAULT_STATUS           = 0x2831,
    OD_INDEX_ERROR_DETAIL           = 0x2832,
    OD_INDEX_FIRMWARE_VERSIONS      = 0x5000,
    OD_INDEX_HSI                    = 0x5002,
    OD_INDEX_HKM_HSI                = 0x3000,
    OD_INDEX_HSI_MEM                = 0x3100,
    OD_INDEX_KEEPER                 = 0x2100,
    OD_INDEX_KEEPER_DIAG            = 0x3001,
    OD_INDEX_KEEPER_ERR_HISTORY     = 0x3011,
    OD_INDEX_MAGNETS                = 0x2300,
    OD_INDEX_MAGNETS_O_DIAG         = 0x3003,
    OD_INDEX_MAGNETS_O_ERR_HISTORY  = 0x3013,
    OD_INDEX_MAGNETS_I_DIAG         = 0x3004,
    OD_INDEX_MAGNETS_I_ERR_HISTORY  = 0x3014,
    OD_INDEX_MEM_CORRUPTER          = 0x5005,
    OD_INDEX_PCONTROL               = 0x5003,
    OD_INDEX_SQNC_CTRL              = 0x4200,
    OD_INDEX_THROTTLE               = 0x4002,
    OD_INDEX_THRUSTER_COMMAND       = 0x4000,
    OD_INDEX_DEBUG_TRACE            = 0x5001,
    OD_INDEX_SW_UPDATE              = 0x5500,
    OD_INDEX_UCV                    = 0x5102,
    OD_INDEX_VALVES                 = 0x2500,
    OD_INDEX_VALVES_DIAG            = 0x3005,
    OD_INDEX_VALVES_ERR_HISTORY     = 0x3015,
    OD_INDEX_CALIB_VALS             = 0x5102,
    OD_INDEX_ERASENOR               = 0x5004,
} USER_OD_INDEX;
#endif
