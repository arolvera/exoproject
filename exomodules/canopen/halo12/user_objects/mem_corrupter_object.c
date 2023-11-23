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
#include <stdint.h>

#include "iacm/iacm.h"
#include "mem_corrupter_object.h"                           // Header for this module
#include "update/update_helper.h"
#include "storage/storage_memory.h"
#include "mem_scrubber.h"
#include "icm/tc_icm.h"
#include "halo12-vorago/canopen/user_object_od_indexes.h"

static CO_ERR MemCorrupterWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR MemCorrupterRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t MemCorrupterSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);

/* FLASH memory lock status (defined in storage_memory.h) */
static efc_lock_bits_t lock_bits;


#if __DEBUG
#define CORRUPTION_TEST_ENALBED 1
#else
#define CORRUPTION_TEST_ENALBED 0
#endif

/** 
 * Power Control User Object
 */
CO_OBJ_TYPE MemCorrupterType = {
    MemCorrupterSize,    /* type function to get object size      */
    0,                   /* type function to control type object  */
    MemCorrupterRead,    /* type function to read object content  */
    MemCorrupterWrite,   /* type function to write object content */
};
#define CO_TMCCONTROL  ((CO_OBJ_TYPE*)&MemCorrupterType)

CO_OBJ_DOM iacm_domain = {
    .Offset = 0,
    .Size = sizeof(uint32_t) * IACM_LAST_ITEM,
    .Start = 0,
};
#define CO_IACM_READ  ((uintptr_t)&iacm_domain)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    MC_SUBIDX_COUNT = 0,
    MC_SUBIDX_MEM_COMP_CORRUPT,  
    MC_SUBIDX_STATUS,
    MC_SUBIDX_FAILED,
    MC_SUBIDX_UD_STAT,
    MC_SUBIDX_IACM,
    MC_SUBIDX_FUNLOCK,    
    MC_SUBIDX_REPAIR_ENA,    
    MC_SUBIDX_LOCKBITS,
    MC_SUBIDX_MS_VERIFY,
    MC_SUBIDX_NVM_CORRUPT,
    MC_SUBIDX_IACM_REGION_ERASE,
    MC_SUBIDX_IACM_GET_CORRUPT_RR,
    MC_SUBIDX_MEM_COMP_CORRUPT_ALL,   //Corrupts all thre components given starting comp
    MC_SUBIDX_EOL,
} OD_MemCorrupter;

/**
 * Write to the Power Control variables file
 * 
 * @param  obj  - object dictionary info
 * @param  node - CO Node info
 * @param  buf  - buffer to write from
 * @param  size - size to write
 * 
 * @return err  - success = CO_ERR_NONE, failure = CO_ERR_TYPE_WR
 */
static CO_ERR MemCorrupterWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    
    if(subidx == MC_SUBIDX_FUNLOCK) {
        uint32_t state = *((uint32_t *) buf);
        if (state == 0){
            write_err = sm_flash_lock(0);
            /* Unlocking the flash also implies that the bootloader and/or the
             * application will be programmed, so set the execution repair
             * to block automatic repair */
            iacm_set(IACM_EXECUTION_REPAIR, EXE_REPAIR_BLOCKED);
        }else{
            sm_flash_lock(1);
        }
    } else if(subidx == MC_SUBIDX_MEM_COMP_CORRUPT) {
#if CORRUPTION_TEST_ENALBED
        int fd = -1;
        write_err = tc_icm_lock();
        if(!write_err) {
            char key = *((uint32_t *) buf);
            fd = storage_memory_open(&key, O_WRONLY);
            if(fd < 0) {
                write_err = __LINE__;
            } else {
                uint8_t junk[512] = {0x4a, 0x55, 0x4e, 0x4b};
                int size = sizeof(junk);
                write_err = storage_memory_write(fd, junk, size);
                write_err = (write_err == size) ? 0:__LINE__;
                storage_memory_close(fd);
            }
            tc_icm_unlock();
        }
#endif
    } else if(subidx == MC_SUBIDX_MEM_COMP_CORRUPT_ALL) {
        //Corrupts all 3 redundant components
#if CORRUPTION_TEST_ENALBED
        int fd = -1;
        write_err = tc_icm_lock();
        if(!write_err) {
            char start_key = *((uint32_t *) buf);
            char fname[FILENAME_LENGTH] = {0};
            
            for(int i = 0; RR_NUM_COPIES; i++){
                component_key_translate(&start_key, fname, i);
                fd = storage_memory_open(fname, O_WRONLY);
                if(fd < 0) {
                    write_err = __LINE__;
                } else {
                    uint8_t junk[512] = {0x4a, 0x55, 0x4e, 0x4b};
                    int size = sizeof(junk);
                    write_err = storage_memory_write(fd, junk, size);
                    write_err = (write_err == size) ? 0:__LINE__;
                    storage_memory_close(fd);
                }   
            }
            tc_icm_unlock();
        }
#endif
    } else if(subidx == MC_SUBIDX_REPAIR_ENA) {
        uint32_t val = *((uint32_t *) buf);
        iacm_set(IACM_EXECUTION_REPAIR, val);
    } else if(subidx == MC_SUBIDX_MS_VERIFY) {
        ms_verify();
    } else if(subidx == MC_SUBIDX_NVM_CORRUPT) {
#if CORRUPTION_TEST_ENALBED
        uint32_t addr = *((uint32_t *) buf);
        write_err = sm_erase16_pages(addr);
#endif
    } else if(subidx == MC_SUBIDX_IACM_REGION_ERASE) {
#if CORRUPTION_TEST_ENALBED
        uint32_t region = *((uint32_t *) buf);
        iacm_corrupt_region(region);
#endif
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;      // return the results
}


/**
 * Memory corrupter read function. Sends either component status (good/ccorrupt)
 * or Failed To Repair status back to the CAN master.
 * 
 * 
 * @param  obj   - object dictionary info
 * @param  node  - CO Node info
 * @param  width - buffer to write from
 * 
 * @return err   - success = CO_ERR_NONE, failure = CO_ERR_TYPE_RD
 */
static CO_ERR MemCorrupterRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case MC_SUBIDX_STATUS:
            // read the component status word (components good/corrupt)
            *((uint32_t *)buf) = iacm_get(IACM_MEM_REGION_STATUS);
            break;
            
        case MC_SUBIDX_FAILED:
            // read the components "failed to repair" status word
            *((uint32_t *)buf) = iacm_get(IACM_FAILED_REPAIRS);
            
            // then zero this status, report only new failures since last read
            iacm_resetbits(IACM_FAILED_REPAIRS, *((uint32_t *) buf));
            break;
            
        case MC_SUBIDX_UD_STAT:
            // read the "Component Just Updated" status
            *((uint32_t *)buf) = iacm_get(IACM_REPAIR_STATUS);
            
            // then zero this status, report only new updates since last read
            iacm_resetbits(IACM_REPAIR_STATUS, *((uint32_t *) buf));
            break;
            
        case MC_SUBIDX_FUNLOCK:
            sm_lock_bits_get(&lock_bits);
            *((uint32_t *)buf) = lock_bits.lock_bits[0];
            break;
            
        case MC_SUBIDX_REPAIR_ENA:
            *((uint32_t *)buf) = iacm_get(IACM_EXECUTION_REPAIR);
            break;
            
        case MC_SUBIDX_LOCKBITS:
            sm_lock_bits_get(&lock_bits);
            memcpy(buf, &lock_bits, size);
            break;            
        
        case MC_SUBIDX_IACM_GET_CORRUPT_RR:
            *((uint32_t *)buf) = iacm_get_crpt_rr_msk();

            break;
        default:
            err = __LINE__; // Not implemented
    }
    
    return err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}


/**
 * Memory corrupter size function. Sets the size of the data transfers.
 * 
 * 
 * @param  obj   - object dictionary info
 * @param  node  - CO Node info
 * @param  width - buffer to write from
 * 
 * @return size  - size of data read
 */
static uint32_t MemCorrupterSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx){
        case MC_SUBIDX_MEM_COMP_CORRUPT:
        case MC_SUBIDX_STATUS:
        case MC_SUBIDX_FAILED:
        case MC_SUBIDX_UD_STAT:
        case MC_SUBIDX_IACM:
        case MC_SUBIDX_FUNLOCK:
        case MC_SUBIDX_REPAIR_ENA:
        case MC_SUBIDX_NVM_CORRUPT:
        case MC_SUBIDX_IACM_REGION_ERASE:
        case MC_SUBIDX_IACM_GET_CORRUPT_RR:
        case MC_SUBIDX_MEM_COMP_CORRUPT_ALL:
        case MC_SUBIDX_MS_VERIFY:
            size = sizeof(uint32_t);
            break;
        case MC_SUBIDX_LOCKBITS:
            size = sizeof(efc_lock_bits_t);
            break;
        default:
            size = 0;
            break;
    }
    return size;
}

void MemCorrupterOD(OD_DYN *self)
{
    iacm_domain.Start = (uint8_t*)iacm_base_addr_get();
    
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_COUNT,
            CO_UNSIGNED8|CO_OBJ_D__R_), 0, MC_SUBIDX_EOL);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_MEM_COMP_CORRUPT,
            CO_UNSIGNED32|CO_OBJ____RW), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_STATUS,
            CO_UNSIGNED32|CO_OBJ____R_), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_FAILED,
            CO_UNSIGNED32|CO_OBJ____R_), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_UD_STAT,
            CO_UNSIGNED32|CO_OBJ____R_), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_IACM,
            CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_IACM_READ);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_FUNLOCK,
            CO_UNSIGNED32|CO_OBJ____RW), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_REPAIR_ENA,
            CO_UNSIGNED32|CO_OBJ____RW), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_LOCKBITS,
            CO_STRING|CO_OBJ____R_), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_MS_VERIFY,
            CO_STRING|CO_OBJ_____W), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_NVM_CORRUPT,
            CO_UNSIGNED32|CO_OBJ_____W), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_IACM_REGION_ERASE,
            CO_UNSIGNED32|CO_OBJ_____W), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_IACM_GET_CORRUPT_RR,
            CO_UNSIGNED32|CO_OBJ____R_), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
    ODAdd(self, CO_KEY(OD_INDEX_MEM_CORRUPTER, MC_SUBIDX_MEM_COMP_CORRUPT_ALL,
            CO_UNSIGNED32|CO_OBJ_____W), CO_TMCCONTROL, (uintptr_t)&MemCorrupterType);
}