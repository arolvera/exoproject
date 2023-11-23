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
 * power_control_object.c
 *
 * @Company Exoterra
 * @File power_control_object.c
 * @Summary  User object for power control, magnet, valves, etc..
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */
#include <string.h>                 // memcpy

#include "power_control_object.h"
#include "client-control/power/client_power.h"                           // Header for this module
#include "user_object_od_indexes.h"
#include "task-monitor/component_tasks.h"
#include "client-control/client_control.h"

static CO_ERR PowerControlWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR PowerControlRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

/** 
 * Power Control User Object
 */
CO_OBJ_TYPE PowerControlType = {
    0,                   /* type function to get object size      */
    0,                   /* type function to control type object  */
    PowerControlRead,    /* type function to read object content  */
    PowerControlWrite,   /* type function to write object content */
};
#define CO_TPCONTROL  ((CO_OBJ_TYPE*)&PowerControlType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    POWERC_SUBIDX_COUNT = 0,
    POWERC_SUBIDX_MAG_VALVE_ANODE,
    POWERC_SUBIDX_KEEPER,
    POWERC_SUBIDX_EOL,
} OD_PowerControl;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * Write to the Power Control variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR PowerControlWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint8_t val = *(uint8_t*) buf;
    bool power_on = val ? true:false; // Any value is on.  Zero is off
    
    switch(subidx) {
        case POWERC_SUBIDX_MAG_VALVE_ANODE:
//            comp_power(MVCP_RESID, power_on);
            client_reset(); // this will also reset the keep
            break;
        case POWERC_SUBIDX_KEEPER:
            comp_power(ECPK_RESID, power_on);   // this will only affect the keeper
            break;
        case POWERC_SUBIDX_COUNT : /* Read-only - fall through to error */
        default:
           write_err = __LINE__;
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the power control variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR PowerControlRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case POWERC_SUBIDX_MAG_VALVE_ANODE:
            *((uint16_t *)buf) = gpio_rd(&gpio_mcu_shutdown);
            break;
        case POWERC_SUBIDX_KEEPER:
            //Only works when using ECPK
            *((uint16_t *)buf) = eTaskGetState(comp_task_list_get()->comp_tasks[1]->comp_task_stack.task_handle);
            break;
        case POWERC_SUBIDX_COUNT :
            *((uint32_t *)buf) = POWERC_SUBIDX_COUNT - 1;
            break;
        default:
            read_err = __LINE__; // Not implemented
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void PowerControlOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_PCONTROL, POWERC_SUBIDX_COUNT,
            CO_UNSIGNED8|CO_OBJ____R_), CO_TPCONTROL,(uintptr_t)&PowerControlType);
    ODAdd(self, CO_KEY(OD_INDEX_PCONTROL, POWERC_SUBIDX_MAG_VALVE_ANODE,
            CO_UNSIGNED16|CO_OBJ____RW), CO_TPCONTROL, (uintptr_t)&PowerControlType);
    ODAdd(self, CO_KEY(OD_INDEX_PCONTROL, POWERC_SUBIDX_KEEPER,
                       CO_UNSIGNED16|CO_OBJ____RW), CO_TPCONTROL, (uintptr_t)&PowerControlType);
}
/* *****************************************************************************
 End of File
 */
