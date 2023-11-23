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
 email:  jknapo@exoterracorp.com
*/
#include "calib_vals_ucv_object.h"
#include "client_control/calib_vals_ucv.h"
#include "halo12-vorago/canopen/user_object_od_indexes.h"

static CO_ERR CalUCVWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR CalUCVRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t CalUCVSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);
CO_OBJ_TYPE CalUCVType = {
    CalUCVSize,    /* type function to get object size      */
    0,             /* type function to control type object  */
    CalUCVRead,    /* type function to read object content  */
    CalUCVWrite,   /* type function to write object content */
};
#define CO_CALUCV  ((CO_OBJ_TYPE*)&CalUCVType)


typedef enum {
    CALIB_VALS_UCV_SUBIDX_COUNT     =  0,
    CALIB_VALS_UCV_VALVE_3000_PSI   =  1,
    CALIB_VALS_UCV_SUBIDX_EOL
} OD_CALIB_VALS_UCV;

/**
 * Write to the valves variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR CalUCVWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    int subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case CALIB_VALS_UCV_VALVE_3000_PSI:
            calib_vals_ucv_cnt_psi_three_thousand_set(*(float *)buf);
            break;
        default:
             write_err = __LINE__; //
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;    
}

/**
 * Write to the valves variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR CalUCVRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    int subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case CALIB_VALS_UCV_VALVE_3000_PSI:
            *((float *)buf) = calib_vals_ucv_cnt_psi_three_thousand_get();
            break;             
        case CALIB_VALS_UCV_SUBIDX_COUNT : 
            *((uint8_t *)buf) = CALIB_VALS_UCV_SUBIDX_EOL;
            break;
        default:
            read_err = __LINE__; // Not implemented
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
static uint32_t CalUCVSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    
    int subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case CALIB_VALS_UCV_SUBIDX_COUNT:
            size = sizeof(uint8_t);
            break;
        case CALIB_VALS_UCV_VALVE_3000_PSI:
            size = sizeof(float);
            break;       
        case CALIB_VALS_UCV_SUBIDX_EOL:
        default:
            size = 0;
            
    }
    return size;
}

void CalibValsUCVOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_CALIB_VALS_UCV, CALIB_VALS_UCV_SUBIDX_COUNT,
            CO_UNSIGNED8 | CO_OBJ____R_), CO_CALUCV, (uintptr_t)&CalUCVType);
    ODAdd(self, CO_KEY(OD_INDEX_CALIB_VALS_UCV, CALIB_VALS_UCV_VALVE_3000_PSI,
            CO_FLOAT | CO_OBJ____RW), CO_CALUCV, (uintptr_t)&CalUCVType);
}

