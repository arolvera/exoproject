//           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
//
// Unauthorized copying of this file, via any medium is strictly prohibited
//                                       Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//    distribution, or disclosure of this software is expressly forbidden.
//
//                     This Copyright notice may not be removed or modified without prior written
//           consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//       ExoTerra Corp
//       7640 S. Alkire Pl.
//       Littleton, CO 80127
//    USA
//
//        Voice:  +1 1 (720) 788-2010
//                   http:   www.exoterracorp.com
//                         email:  contact@exoterracorp.com

//
// erase_nor_flash_object.c
//
// @Company Exoterra
// @File keeper_object.c
// @Summary  User object for keeper control.
//
// For details, see the canopen-stack documentation here:
// https://canopen-stack.org/docs/usecase/dictionary
//

#include "erase_nor_flash_object.h"
#include "flash/hal_flash.h"
#include "user_object_od_indexes.h"

static CO_ERR EraseNorWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR EraseNorRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

// Nor Erase User Object
CO_OBJ_TYPE EraseNorType = {
    0,             // type function to get object size
    0,             // type function to control type object
    EraseNorRead,  // type function to read object content
    EraseNorWrite, // type function to write object content
};

#define CO_TERASENOR  ((CO_OBJ_TYPE*)&EraseNorType)

// SUB-Indexes - Must match this order in the Object Dictionary
typedef enum {
    ERASENOR_SUBIDX_COUNT,
    ERASENOR_SUBIDX_ALL,
    ERASENOR_SUBIDX_SPECIFIC,
    ERASENOR_SUBIDX_EOL,
} OD_ERASENOR;

/**
 * Write to the erasenor variables file
 *
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR EraseNorWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size) {
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    switch (subidx) {
        case ERASENOR_SUBIDX_ALL:
            for (size_t i = 0; i < NUM_SECTORS; i++) {
                if (flash_sector_erase(i)) { write_err = 1; } // Error
            }
            break;
        case ERASENOR_SUBIDX_SPECIFIC:
            if (*((uint16_t *)buf) >= NUM_SECTORS) { write_err = 1; }
            else { write_err = flash_sector_erase(*((uint16_t *)buf)); }
            break;
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

static CO_ERR EraseNorRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size) {
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    switch (subidx) {
        case ERASENOR_SUBIDX_ALL:
            for (size_t i = 0; i < NUM_SECTORS; i++) {
                if (flash_sector_blank_check(i)) { write_err = 1; }
            }
            break;
        case ERASENOR_SUBIDX_SPECIFIC:
            if (*((uint16_t *)buf) >= NUM_SECTORS) { write_err = 1; }
            else { write_err = flash_sector_blank_check(*((uint16_t *)buf)); }
            break;
        case ERASENOR_SUBIDX_COUNT :
            *((uint32_t *)buf) = ERASENOR_SUBIDX_EOL-1;
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

void EraseNorOD(OD_DYN *self) {
    ODAdd(self, CO_KEY(OD_INDEX_ERASENOR, ERASENOR_SUBIDX_COUNT, CO_UNSIGNED16  |CO_OBJ_D__R_), 0, ERASENOR_SUBIDX_EOL - 1);
    ODAdd(self, CO_KEY(OD_INDEX_ERASENOR, ERASENOR_SUBIDX_ALL, CO_UNSIGNED16 | CO_OBJ____RW), CO_TERASENOR, (uintptr_t)&EraseNorType);
    ODAdd(self, CO_KEY(OD_INDEX_ERASENOR, ERASENOR_SUBIDX_SPECIFIC, CO_UNSIGNED16 | CO_OBJ____RW), CO_TERASENOR, (uintptr_t)&EraseNorType);
}
