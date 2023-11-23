/**
 * @file    update_object.c
 *
 * @brief   User object function definitions for receiving a software update
 * file via the CAN Open interface.  The functions here are called when the
 * user accesses the corresponding Object Dictionary item.
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

#include "update_object.h"// Header for this module
#include "definitions.h"
#include "trace/trace.h"
#include "update/update_command.h"
#include "update/update_helper.h"
#include "user_object_od_indexes.h"
#include "client-control/power/client_power.h"



static uint32_t verified = 0;

typedef enum {
    SW_UPDATE_SUBINDEX_COUNT         = 0,
    SW_UPDATE_SUBINDEX_UPLOAD        = 1,
    SW_UPDATE_SUBINDEX_VERIFY        = 2,
    SW_UPDATE_SUBINDEX_INSTALL       = 3,
    SW_UPDATE_SUBINDEX_ENABLE        = 4,
    SW_UPDATE_SUBINDEX_EOL,
} OD_SW_UPDATE;



static uint32_t UpdateSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);
static CO_ERR   UpdateWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   UpdateRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

CO_OBJ_TYPE UpdateType = {
    UpdateSize,  /* type function to get object size         */
    0,           /* type function to control type object     */
    UpdateRead,  /* type function to read object content     */
    UpdateWrite  /* type function to write object content    */
};
#define CO_TUPDATE  ((CO_OBJ_TYPE*)&UpdateType)



/**
 * @brief Size callback for the Anode user object. The CAN Open stack needs to know the
 * size of each object when reading. When writing, the stack may provide the number of
 * bytes to be written?
 * @param obj object dictionary info
 * @param node  CO Node info
 * @param width file size
 * @return CO_ERR_NONE on success, or 0 if transfer already in progress, which
 * will abort the new transfer request
 */
static uint32_t UpdateSize (struct CO_OBJ_T * obj, struct CO_NODE_T *node, uint32_t width)
{
    int err = 0;
    uint32_t ret = CO_ERR_NONE;
    uint8_t subidx = CO_GET_SUB(obj->Key);

    switch(subidx) {
        case SW_UPDATE_SUBINDEX_UPLOAD:
            verified = 0;
            if(Thruster_NMT_State != CO_PREOP){
                err = CO_ERR_NMT_MODE;
            } else {
                err = uc_prepare(width);        // This opens a file pointer to the sram update region
                if(err) {
                    ret = CO_ERR_OBJ_SIZE;
                } else {
                    /* return back the requested size */
                    ret = width;
                }
            }
            break;

        case SW_UPDATE_SUBINDEX_ENABLE:
        case SW_UPDATE_SUBINDEX_INSTALL:
            ret = sizeof(uint8_t);
            break;

        case SW_UPDATE_SUBINDEX_COUNT:
        case SW_UPDATE_SUBINDEX_VERIFY:
            ret = sizeof(uint32_t);             /* All unsigned 32 bits */
            break;

        default:
            ret = 0;
    }
    return ret;
}
/**
 * @brief Write to the update file
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size number of bytes to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR UpdateWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int err = CO_ERR_NONE;
    uint8_t subidx = CO_GET_SUB(obj->Key);

    if(Thruster_NMT_State != CO_PREOP){
        err = CO_ERR_NMT_MODE;
    } else {
        if (subidx == SW_UPDATE_SUBINDEX_UPLOAD) {
            err = uc_upload(buf, size);

        } else if (subidx == SW_UPDATE_SUBINDEX_VERIFY) {
            /* Update with 'NONE' only verifies and fills in the mask */
            err = uc_verify();
            if (!err) { verified = 1; }

        } else if (subidx == SW_UPDATE_SUBINDEX_INSTALL) {
            err = uc_install();
        } else if (subidx == SW_UPDATE_SUBINDEX_ENABLE) {
            cp_reset_all();     // reset the clients instead of power down
            __NVIC_SystemReset();
        } else {
            err = CO_ERR_SDO_ABORT;
        }
        if (!err) {
            err = CO_ERR_NONE;
        } else if (err != CO_ERR_SDO_ABORT) {
            err = CO_ERR_TYPE_WR;
        }
    }
//    TraceDbg(TrcMsgUpdate, "error:%x si:%x we:%x ae:%x",
//            err, subidx, CO_ERR_TYPE_WR, CO_ERR_SDO_ABORT,0,0);
    return err;
}

static CO_ERR UpdateRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case SW_UPDATE_SUBINDEX_COUNT:
            *((uint32_t *)buf) = SW_UPDATE_SUBINDEX_EOL - 1;
            break;
        case SW_UPDATE_SUBINDEX_VERIFY:
            *((uint32_t *)buf) = verified;
            break;
        default :
            err = __LINE__;
    }
    return err;
}

void UpdateOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_SW_UPDATE, SW_UPDATE_SUBINDEX_COUNT,  CO_UNSIGNED8 |CO_OBJ____R_), CO_TUPDATE, (uintptr_t)&UpdateType);
    ODAdd(self, CO_KEY(OD_INDEX_SW_UPDATE, SW_UPDATE_SUBINDEX_UPLOAD,CO_UNSIGNED32|CO_OBJ_____W), CO_TUPDATE, (uintptr_t)&UpdateType);
    ODAdd(self, CO_KEY(OD_INDEX_SW_UPDATE, SW_UPDATE_SUBINDEX_VERIFY, CO_UNSIGNED32|CO_OBJ____RW), CO_TUPDATE, (uintptr_t)&UpdateType);
    ODAdd(self, CO_KEY(OD_INDEX_SW_UPDATE, SW_UPDATE_SUBINDEX_INSTALL,CO_UNSIGNED8 |CO_OBJ_____W), CO_TUPDATE, (uintptr_t)&UpdateType);
    ODAdd(self, CO_KEY(OD_INDEX_SW_UPDATE, SW_UPDATE_SUBINDEX_ENABLE,CO_UNSIGNED8 |CO_OBJ_____W), CO_TUPDATE, (uintptr_t)&UpdateType);
}
