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

#include <string.h>

#include "trace_object.h"                           // Header for this module
#include "trace/trace.h"                   // Trace globals
#include "user_object_od_indexes.h"

#if _TRACE_ENABLE

static CO_ERR TraceDebugWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR TraceDebugRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t TraceDebugSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);

/**
 * Trace Control User Object
 */
CO_OBJ_TYPE TraceType = {
    TraceDebugSize,    /* type function to get object size      */
    0,                 /* type function to control type object  */
    TraceDebugRead,    /* type function to read object content  */
    TraceDebugWrite,                 /* type function to write  (Read-only)   */
};
#define CO_TTRACE  ((CO_OBJ_TYPE*)&TraceType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    TRACE_SUBIDX_COUNT = 0, 
    TRACE_SUBIDX_FLAG,
    TRACE_SUBIDX_HEAD,
    TRACE_SUBIDX_TAIL,
    TRACE_SUBIDX_SIZE,
    TRACE_SUBIDX_MSG_HD,
    TRACE_SUBIDX_MSG_TL,
    TRACE_SUBIDX_PEEK_ADR,
    TRACE_SUBIDX_PEEK_VAL,
    TRACE_SUBIDX_POKE_VAL,
    TRACE_SUBIDX_LOCKOUT_OVERRIDE,
    TRACE_SUBIDX_EOL,
} OD_TRACE;


#define LOCKOUT_OVERRIDE 0x6f6c6466 /* 'fdlo' */

#define MAX_TRACE_MSG_SIZE 128
static char trace_message[MAX_TRACE_MSG_SIZE] = {0};
static int pos = 0;

static uint32_t addr = 0x440000; /* Flash start, because thats what i am working now */// ToDo: isn't this no-mans land?

/**
 * Size callback for user object.  The CAN Open stack needs to know the size
 * of each object
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return size of the field in bytes
 */
static uint32_t TraceDebugSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    int err = 0;
    int which_end = 0;
    
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    if(subidx == TRACE_SUBIDX_MSG_HD || subidx == TRACE_SUBIDX_MSG_TL) {
        if(subidx == TRACE_SUBIDX_MSG_TL) {
            which_end = 1;
        }
        /* Go ahead and get the message now.  The next call to read will 
           return this message   */
        size = trcGet(trace_message, MAX_TRACE_MSG_SIZE, which_end);
        /* Reset pos */
        pos = 0;
    } else if(subidx == TRACE_SUBIDX_PEEK_ADR ||
              subidx == TRACE_SUBIDX_PEEK_VAL ||
              subidx == TRACE_SUBIDX_POKE_VAL || 
              subidx == TRACE_SUBIDX_LOCKOUT_OVERRIDE) {
        size = 4; // unsigned 32 bit int
    } else {
        err = -1;
    }
    
    return err ? 0:size; // Return size zero on error, else return the size
}

/**
 * Read the valves variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR TraceDebugRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    if(size + pos > MAX_TRACE_MSG_SIZE) {
        read_err = -1;
    } else {
        switch(subidx) {
            case TRACE_SUBIDX_MSG_HD :
            case TRACE_SUBIDX_MSG_TL :
                memcpy(buf, trace_message + pos, size);
                pos += size;
                break;
           case TRACE_SUBIDX_PEEK_VAL :
               /* always make sure addr is 32 bit aligned */
               addr &= ~0x3;
               /* auto increment address whenever the value is read*/
               *((uint32_t *) buf) = *((uint32_t *)addr);
               addr += 4;
               break;
            default:
                read_err = __LINE__; // Not implemented
        }
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/**
 * Write to ram
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR TraceDebugWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint32_t val = 0;

    switch(subidx) {
        case TRACE_SUBIDX_POKE_VAL:
            /* always make sure addr is 32 bit aligned */
            addr &= ~0x3;
            val = *((uint32_t *)buf);
            *((uint32_t *)addr) = val;
            addr += 4;
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


void TraceDebugOD(OD_DYN *self)
{
    /* Direct access */
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_COUNT,CO_UNSIGNED8  | CO_OBJ_D__R_), 0, TRACE_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_FLAG, CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&trcMsg);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_HEAD, CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&traceHd);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_TAIL, CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&traceTl);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_SIZE, CO_UNSIGNED32 | CO_OBJ_D__R_), 0, TraceSize);
    
    /* Special handling */
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_MSG_HD, CO_STRING|CO_OBJ____R_), CO_TTRACE, (uintptr_t)&TraceType);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_MSG_TL, CO_STRING|CO_OBJ____R_), CO_TTRACE, (uintptr_t)&TraceType);
    
    /* Just a variable */
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_PEEK_ADR, CO_UNSIGNED32 | CO_OBJ____RW), 0, (uintptr_t)&addr);
    /* More Special handling */
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_PEEK_VAL, CO_STRING | CO_OBJ____R_), CO_TTRACE, (uintptr_t)&TraceType);
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_POKE_VAL, CO_STRING | CO_OBJ_____W), CO_TTRACE, (uintptr_t)&TraceType);
    
    /* Because the lockout time is way too long for development */
    ODAdd(self, CO_KEY(OD_INDEX_DEBUG_TRACE, TRACE_SUBIDX_LOCKOUT_OVERRIDE, CO_UNSIGNED32 | CO_OBJ_____W), CO_TTRACE, (uintptr_t)&TraceType);
    
}
#else
void TraceDebugOD(OD_DYN *self) {}
#endif
/* *****************************************************************************
 End of File
 */
