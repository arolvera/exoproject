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

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "user_config_var_object.h"
#include "client_control/client_control_ucv.h"
#include "user_conf_vars/user_conf_vars.h"
#include "halo12-vorago/canopen/user_object_od_indexes.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

static CO_ERR UserConfigVarStore(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);


/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    USERCONF_SUBIDX_COUNT = 0,
    USERCONF_STORE,
    USERCONF_ERASE,
    USERCONF_SUBIDX_EOL,
} OD_UserConfVar;

/** 
 * User Conf Var Control User Object
 */
CO_OBJ_TYPE UserConfigVarTypes = {
    0,                   /* type function to get object size      */
    0,                   /* type function to control type object  */
    0,                   /* type function to control type object  */
    UserConfigVarStore,   /* type function to write object content */
};
#define CO_USERCONFVARTYPE  ((CO_OBJ_TYPE*)&UserConfigVarTypes)

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * Write to the User Conf Var variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR UserConfigVarStore(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint8_t el =  (size == 0) ? 0xFF : ((uint8_t*)buf)[0];
    
    switch(subidx) {
	case USERCONF_STORE:
	    ucv_store();
	    break;
	case USERCONF_ERASE:
            ucv_clear(el);
            break;
        default:
           write_err = __LINE__;
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void UserConfigVarOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_UCV, USERCONF_SUBIDX_COUNT, 
	CO_SIGNED32|CO_OBJ_____W), CO_USERCONFVARTYPE,(uintptr_t)&UserConfigVarTypes);
    ODAdd(self, CO_KEY(OD_INDEX_UCV, USERCONF_STORE, 
	CO_SIGNED32|CO_OBJ_____W), CO_USERCONFVARTYPE,(uintptr_t)&UserConfigVarTypes);
    ODAdd(self, CO_KEY(OD_INDEX_UCV, USERCONF_ERASE, 
	CO_SIGNED32|CO_OBJ_____W), CO_USERCONFVARTYPE,(uintptr_t)&UserConfigVarTypes);
}
/* *****************************************************************************
 End of File
 */
