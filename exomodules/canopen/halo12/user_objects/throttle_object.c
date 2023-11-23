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
 * throttle_object.c
 *
 * @Company Exoterra
 * @File throttle_object.c
 * @Summary  User object for anode control.
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <string.h>                 // memcpy

#include "throttle_object.h"                           // Header for this module
#include "sequence/thruster-start/control_thruster_start.h"
#include "thruster_control.h"
#include "user_object_od_indexes.h"
#include "setpoint/control_setpoint.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */ 
/* ************************************************************************** */
/* ************************************************************************** */

static CO_ERR   ThrottleWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   ThrottleRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t ThrottleSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);
static uint32_t table_selection = 0;
static uint32_t set_point = 0;

/** 
 * Throttle Control User Object
 */
CO_OBJ_TYPE ThrottleType = {
    ThrottleSize,    /* type function to get object size      */
    0,            /* type function to control type object  */
    ThrottleRead,    /* type function to read object content  */
    ThrottleWrite,   /* type function to write object content */
};
#define CO_TTHROTTLE  ((CO_OBJ_TYPE*)&ThrottleType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    THROTTLE_SUBINX_HIGHEST_SUBINDEX,
    THROTTLE_SUBINX_TABLE_SELECTOR,
    THROTTLE_SUBINX_ROW_SELECTOR,
    THROTTLE_SUBIDX_CATHODE,
    THROTTLE_SUBIDX_ANODE,
    THROTTLE_SUBIDX_VOLTAGE,
    THROTTLE_SUBIDX_CURRENT,
    THROTTLE_SUBIDX_INNER,
    THROTTLE_SUBIDX_O_I,
    THROTTLE_SUBIDX_THRUST,
    THROTTLE_SUBIDX_POWER,
    THROTTLE_SUBIDX_START_METHOD,
    THROTTLE_SUBIDX_TIMEOUT,
    THROTTLE_SUBIDX_SETPOINT, 
    THROTTLE_SUBIDX_TABLE_LEN,
    THROTTLE_SUBIDX_EOL,
} OD_THROTTLE;


/**
 * Write to the throttle variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ThrottleWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case THROTTLE_SUBINX_TABLE_SELECTOR:
            table_selection = (*((uint32_t *)buf));
            break;
        case THROTTLE_SUBINX_ROW_SELECTOR:
            set_point = (*((uint32_t *)buf)) - 1;
            break;
        case THROTTLE_SUBIDX_CATHODE :
            write_err = thrust_cathode_lf_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_ANODE : 
            write_err = thrust_anode_flow_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_VOLTAGE :
            write_err = thrust_anode_v_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_CURRENT :
            write_err = thrust_anode_i_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_INNER :
            write_err = thrust_magnet_i_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_O_I :
            write_err = thrust_magnet_ratio_setpoint_set(set_point ,*((float *)buf));
            break;
        case THROTTLE_SUBIDX_THRUST:
            write_err = thrust_millinewtons_set(set_point, *((float *)buf));
            break;
        case THROTTLE_SUBIDX_POWER:
            write_err = thrust_power_set(set_point, *((float *)buf));
            break;
        case THROTTLE_SUBIDX_START_METHOD:
            write_err = thrust_start_method_set(set_point, *((start_method_t *)buf));
            break;
        case THROTTLE_SUBIDX_TIMEOUT:
            write_err = thrust_timeout_set(set_point, *((uint32_t *)buf));
            break;
        case THROTTLE_SUBIDX_SETPOINT:
            write_err = thrust_hf_start_setpoint_set(set_point, *((uint32_t *)buf));
            break;
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the anode variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ThrottleRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    uint16_t idx    = CO_GET_IDX(obj->Key);
    
    if(idx == OD_INDEX_THROTTLE){
        switch(subidx) {
            case THROTTLE_SUBINX_TABLE_SELECTOR :
                *((uint32_t *)buf) = table_selection;
                break;
            case THROTTLE_SUBINX_ROW_SELECTOR :
                *((uint32_t *)buf) = set_point + 1;
                break;
            case THROTTLE_SUBIDX_CATHODE :
                read_err = thrust_cathode_lf_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_ANODE :
                read_err = thrust_anode_flow_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_VOLTAGE :
                read_err = thrust_anode_v_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_CURRENT :
                read_err = thrust_anode_i_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_INNER :
                read_err = thrust_magnet_i_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_O_I :
                read_err = thrust_magnet_ratio_setpoint_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_THRUST:
                read_err = thrust_millinewtons_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_POWER:
                read_err = thrust_power_get(set_point, ((float *)buf));
                break;
            case THROTTLE_SUBIDX_START_METHOD:
                read_err = thrust_start_method_get(set_point, ((start_method_t *)buf));
                break;
            case THROTTLE_SUBIDX_TIMEOUT:
                read_err = thrust_timeout_get(set_point, ((uint32_t *)buf));
                break;
            case THROTTLE_SUBIDX_SETPOINT:
                read_err = thrust_hf_start_setpoint_get(set_point, ((uint32_t *)buf));
                break;
            case THROTTLE_SUBIDX_TABLE_LEN:
                *((uint32_t *)buf) = thrust_table_max_valid();
                break;
            default:
                read_err = __LINE__; // Not implemented
        }
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/**
 * Size callback for user object.  The CAN Open stack needs to know the size
 * of each object
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return size of the field in bytes
 */
static uint32_t ThrottleSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case THROTTLE_SUBINX_TABLE_SELECTOR :
            size = sizeof(table_selection);
            break;
        case THROTTLE_SUBINX_ROW_SELECTOR :
            size = sizeof(set_point);
            break;
        case THROTTLE_SUBIDX_CATHODE:
        case THROTTLE_SUBIDX_ANODE:
        case THROTTLE_SUBIDX_VOLTAGE:
        case THROTTLE_SUBIDX_CURRENT:
        case THROTTLE_SUBIDX_INNER:
        case THROTTLE_SUBIDX_O_I:
        case THROTTLE_SUBIDX_THRUST:
        case THROTTLE_SUBIDX_POWER:
            size = sizeof(float);
            break;
        case THROTTLE_SUBIDX_START_METHOD:
        case THROTTLE_SUBIDX_TIMEOUT:
        case THROTTLE_SUBIDX_SETPOINT:
        case THROTTLE_SUBIDX_TABLE_LEN:
            size = sizeof(uint32_t);
            break;
        default:
            size = 0; // Not implemented
    }
    return size;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
void ThrottleOD(OD_DYN *self)
{    
    /* Thruster Control */
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBINX_HIGHEST_SUBINDEX,
                CO_UNSIGNED8|CO_OBJ_D__R_), 0, THROTTLE_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBINX_TABLE_SELECTOR,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBINX_ROW_SELECTOR,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_CATHODE,
                CO_FLOAT     |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_ANODE,
                CO_FLOAT|CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_VOLTAGE,
                CO_FLOAT     |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_CURRENT,
                CO_FLOAT|CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_INNER,
                CO_FLOAT |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_O_I,
                CO_FLOAT |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_THRUST,
                CO_FLOAT |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_POWER,
                CO_FLOAT |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_START_METHOD,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_TIMEOUT,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_SETPOINT,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
    ODAdd(self, CO_KEY(OD_INDEX_THROTTLE, THROTTLE_SUBIDX_TABLE_LEN,
                CO_UNSIGNED32 |CO_OBJ____R_), CO_TTHROTTLE, (uintptr_t)&ThrottleType);
}
/* *****************************************************************************
 End of File
 */
