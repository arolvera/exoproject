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

#include <stdint.h>
#include <strings.h>
#include <string.h>
#include "checksum.h"
#include "iacm.h"

#ifdef FREE_RTOS
//Abstract mutex use for environment that do not have freertos.
//Such as the bootloader
#pragma message ( "Using Locking Scheme 1 - OSAL_MUTEX" )
#include "definitions.h"
static OSAL_STATIC_MUTEX_BUF  iacm_mtx_buf;
static OSAL_MUTEX_HANDLE_TYPE iacm_mtx;
#define IACM_CREATE_MTX()         OSAL_MUTEX_Create(&iacm_mtx, &iacm_mtx_buf, "iacm")
#define IACM_LOCK_MTX_WAIT()      OSAL_MUTEX_Lock(&iacm_mtx, OSAL_WAIT_FOREVER)
#define IACM_LOCK_MTX_NO_WAIT()   (OSAL_MUTEX_Lock(&iacm_mtx, 0) == OSAL_RESULT_TRUE)
#define IACM_UNLOCK_MTX()         OSAL_MUTEX_Unlock(&iacm_mtx)

#elif
#pragma message ( "Using Locking Scheme 2 - No MUTEX" )

#define IACM_LOCK_MTX_NO_WAIT()   (1)
#define IACM_CREATE_MTX()
#define IACM_LOCK_MTX_WAIT()
#define IACM_UNLOCK_MTX()
#else
#error "You must specify which locking functions you are using"
#endif

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif //ENABLE_DEBUG

// Inter Application Communication Memory
#define IACM_SRAM_START     0x2045F400      // SRAM reserved for IACM
#define IACM_SRAM_END       0x2045FFFF
#define IACM_COPY_END       1023
#define IACM_COPY_SIZE      1024
#define IACM_NUM_COPIES     3
#define IACM_CRC_LOCATION   (IACM_COPY_SIZE - sizeof(uint32_t)) // Using crc16
#define IACM_SIZE           (IACM_COPY_SIZE * IACM_NUM_COPIES) // 3 copies => 3KB total
#define IACM_COPY0_BASE     0x2045F400      // 1KB
#define IACM_COPY0_END      IACM_COPY0_BASE + IACM_COPY_END
#define IACM_COPY1_BASE     0x2045F800      // 1KB
#define IACM_COPY1_END      IACM_COPY1_BASE + IACM_COPY_END
#define IACM_COPY2_BASE     0x2045FC00      // 1KB
#define IACM_COPY2_END      IACM_COPY2_BASE + IACM_COPY_END

//Used to note that a region has be found as corrupt. 
#define IACM_SET_CORRUPT_REGION(__rr__) (1 << __rr__)
//Bit mask representing all regions corrupt
#define IACM_ALL_REGION_CORRUPT() ((1 << IACM_NUM_COPIES) - 1)
//Used for debugging corrupt regions
static uint32_t iacm_found_crpt_rr_dbg_msk = 0;

#define IACM_QUEUE_SIZE 32
typedef struct {
    IACM_ITEM_t item;
    uint32_t val;
}iacm_queued_write_t;

typedef struct {
    iacm_queued_write_t qrw[IACM_QUEUE_SIZE];
    uint16_t queue_hd;
    uint16_t queue_tl;
}iacm_queue_t;

static volatile iacm_queue_t iacm_queue = {0};

#define IACM_CRC_CALC(__iacm_mem_layout__) (crc_16((uint8_t *) __iacm_mem_layout__, (IACM_CRC - 1) * sizeof(uint32_t))) 

static uint32_t *iacm_layout[] = {
    (uint32_t *)IACM_COPY0_BASE,
    (uint32_t *)IACM_COPY1_BASE,
    (uint32_t *)IACM_COPY2_BASE,
};

/**
 * @brief push an item and val into to queue 
 * 
 * @param item 
 * @param val 
 */
static void iacm_queue_push(IACM_ITEM_t item, uint32_t val)
{
    iacm_queue.queue_hd &= IACM_QUEUE_SIZE - 1;
    iacm_queue.qrw[iacm_queue.queue_hd].item = item;
    iacm_queue.qrw[iacm_queue.queue_hd].val = val;
    iacm_queue.queue_hd = (iacm_queue.queue_hd + 1) & (IACM_QUEUE_SIZE - 1);
    if(iacm_queue.queue_hd == iacm_queue.queue_tl)
    {
        iacm_queue.queue_tl = (iacm_queue.queue_tl + 1) & (IACM_QUEUE_SIZE - 1);
    }
}

/**
 * @brief Pops a write off of the queue
 * 
 * @param queued_write 
 * @return uint32_t 0 succ, -1 empty
 */
static uint32_t iacm_queue_pop(iacm_queued_write_t *queued_write)
{
    int err = 0;
    if(iacm_queue.queue_tl != iacm_queue.queue_hd){
        queued_write->item = iacm_queue.qrw[iacm_queue.queue_hd].item;
        queued_write->val = iacm_queue.qrw[iacm_queue.queue_hd].val;
        iacm_queue.queue_hd = (iacm_queue.queue_hd - 1) & (IACM_QUEUE_SIZE - 1);
    }else{
        err = -1;
    }
    return err;
}

/**
 * @brief Clears all iacm regions
 * 
 */
static void iacm_zero(void)
{
    bzero((uint32_t*)IACM_SRAM_START, IACM_SIZE);
}

/**
 * @brief Verifies to crc of each RR. Copies the first 
 * valid region to the corrupt regions.
 * 
 * @return -1 no valid cpy found, o/w valid cpy index into iacm_layout
 */
static int iacm_verify(void)
{
    uint32_t crc = 0;
    uint32_t iacm_crpt_rr_msk = 0;
    int valid_cpy = -1;

    for(int i = 0; i < IACM_NUM_COPIES; i++){
        crc = IACM_CRC_CALC(iacm_layout[i]);
        //Mark corrupt region if crc inval
        if(crc != iacm_layout[i][IACM_CRC]){
            iacm_crpt_rr_msk |= IACM_SET_CORRUPT_REGION(i);
            iacm_found_crpt_rr_dbg_msk |= IACM_SET_CORRUPT_REGION(i);
        }else{
            //Find first good region and get ptr to it
            if(valid_cpy == -1){
                valid_cpy = i;
            }
        }
    }
    
    //Check that there is 
    if(valid_cpy > -1){
        for(int i = 0; i < IACM_NUM_COPIES; i++){
           if((iacm_crpt_rr_msk >> i) & 0x1){
                memcpy(iacm_layout[valid_cpy], iacm_layout[i] , IACM_COPY_SIZE);
           }
        }
    }else{
        iacm_zero();
    }

    return valid_cpy;
}

/**
 * @brief copies valid redundant region to other 2 region
 * 
 * @param valid_cpy 
 */
static void iacm_sync(int valid_cpy)
{
    if((valid_cpy > 0) && (valid_cpy < IACM_NUM_COPIES)){
        //Copy valid copy to other redundant regions
        memcpy(iacm_layout[valid_cpy],
            iacm_layout[(valid_cpy + 1) % IACM_NUM_COPIES], IACM_COPY_SIZE);
        memcpy(iacm_layout[valid_cpy],
            iacm_layout[(valid_cpy + 2) % IACM_NUM_COPIES], IACM_COPY_SIZE);
    }
}

/**
 * @brief Validate item. Then find valid iacm redundent region.
 * Set item. Update crc. Copy IACM redundent region to other regions
 * 
 * @param item 
 * @param val 
 * @return uint32_t 
 */
static uint32_t iacm_set_helper(IACM_ITEM_t item, uint32_t val)
{
    int err = 0;
    int valid_cpy = -1;
    uint32_t *iacm_mem = NULL;

    if(item > IACM_CRC){
        err = -1;
    }else{
        valid_cpy = iacm_verify();
        if(valid_cpy < 0){
            //clear iacm mem region. Then set no valid region
            err = -1;
        }else{
            iacm_mem = iacm_layout[valid_cpy];
            iacm_mem[item] = val;
            iacm_mem[IACM_CRC] = IACM_CRC_CALC(iacm_mem);
            //Update other two iacm copies
            iacm_layout[(valid_cpy + 1) % IACM_NUM_COPIES][item] = iacm_mem[item];
            iacm_layout[(valid_cpy + 2) % IACM_NUM_COPIES][item] = iacm_mem[item];
            iacm_layout[(valid_cpy + 1) % IACM_NUM_COPIES][IACM_CRC] = iacm_mem[IACM_CRC];
            iacm_layout[(valid_cpy + 2) % IACM_NUM_COPIES][IACM_CRC] = iacm_mem[IACM_CRC];
        }
    }
    return err;
}

/**
 * @brief Sets or resets bit
 * 
 * @param item - IACM_ITEM_t
 * @param val - val to set item
 * @param set - 0 set; 1 set
 * @return int err
 */
static int iacm_bit_manager(IACM_ITEM_t item, uint32_t val, int set)
{
    int err = 0;
    int valid_cpy = -1;
    uint32_t *iacm_mem = NULL;

    IACM_LOCK_MTX_WAIT();

    if(item > IACM_CRC){
        err = -1;
    }else{
        valid_cpy = iacm_verify();
        if(valid_cpy < 0){
            err = -1;
        }else{
            iacm_mem = iacm_layout[valid_cpy];
            if(!set){
                iacm_mem[item] &= ~val;
            } else {
                iacm_mem[item] |= val;
            }
            iacm_mem[IACM_CRC] = IACM_CRC_CALC(iacm_mem);

            iacm_layout[(valid_cpy + 1) % IACM_NUM_COPIES][item] = iacm_mem[item];
            iacm_layout[(valid_cpy + 2) % IACM_NUM_COPIES][item] = iacm_mem[item];
            iacm_layout[(valid_cpy + 1) % IACM_NUM_COPIES][IACM_CRC] = iacm_mem[IACM_CRC];
            iacm_layout[(valid_cpy + 2) % IACM_NUM_COPIES][IACM_CRC] = iacm_mem[IACM_CRC];
        }
    }
     
    IACM_UNLOCK_MTX();
    return err;
}

/**
 * @brief Verify then find valid copy.
 * 
 */
void iacm_init(void)
{
    IACM_CREATE_MTX();
//    iacm_verify();
}

/**
 * @brief Checks the validatiy of each iacm region. If there is a
 * valid region. Pop a set off of the queue.
 * 
 * @return err
 */
uint32_t iacm_integrity_check(void)
{
    int err = 0;
    int valid_cpy = 0;
    
    if(IACM_LOCK_MTX_NO_WAIT()){
        valid_cpy = iacm_verify();
        if(valid_cpy >= 0){
            iacm_sync(valid_cpy);
            iacm_queued_write_t queued_write;
            err = iacm_queue_pop(&queued_write);
            if(!err){
                err = iacm_set_helper(queued_write.item, queued_write.val);
            }
        }
        IACM_UNLOCK_MTX();
    }
    return err;
}

/**
 * Writes the IACM 'item' with value 'val'. A new CRC is generated and then
 * the new values are copied across the other 2 IACM copies.
 * 
 * 
 * @param item - index of item to write
 * @param val  - value of item to write
 * @return -1 err finding valid region. 0 o/w
 */
uint32_t iacm_set(IACM_ITEM_t item, uint32_t val)
{
    int err = 0;
    IACM_LOCK_MTX_WAIT();
    err = iacm_set_helper(item, val);
    IACM_UNLOCK_MTX();
    return err;
return 0;
}


/**
 * Does not block on mutex when doing a set. 
 * @param item
 * @param val
 */
void iacm_set_from_fault(IACM_ITEM_t item, uint32_t val)
{
    iacm_set_helper(item, val);
}

/**
 * Queues a write in an interupt. iacm_integrity_check will execute the write 
 * later.  
 * @param item
 * @param val
 */
void iacm_set_from_isr(IACM_ITEM_t item, uint32_t val)
{
    iacm_queue_push(item, val);
}

/**
 * @brief get iacm item val
 * 
 * @param item 
 * @return uint32_t val. -1 O/W
 */
uint32_t iacm_get(IACM_ITEM_t item)
{
    uint32_t val = 0;
    int valid_cpy = -1;

    IACM_LOCK_MTX_WAIT();
    valid_cpy = iacm_verify();

    if(valid_cpy >= 0){
        val = iacm_layout[valid_cpy][item];
    }else{
        val = -1;
    }

    IACM_UNLOCK_MTX();
    return val;
}

/**
 * Transfers the bits set (1) in 'sbits' to the IACM 'item'. Bits are ORed in
 * leaving other bits unaltered.
 * 
 * @param item  - index the of item to modify
 * @param sbits - bits to set in IACM 'item'
 */
int iacm_setbits(IACM_ITEM_t item, uint32_t sbits)
{
    return iacm_bit_manager(item, sbits, 1);
}

/**
 * Transfers the bits set (1) in 'rsbits' to the IACM 'item' as reset (0). Bits
 * are ANDed off leaving other bits unaltered.
 * 
 * @param item   - index of the item to modify
 * @param rsbits - bits to reset in IACM 'item'
 */
int iacm_resetbits(IACM_ITEM_t item, uint32_t rsbits)
{
    return iacm_bit_manager(item, rsbits, 0);
}

/**
 * Tests a bit indicator 'item' for set bits found in 'tbits'. Returns bits as
 * found in the item of copy 0.
 * 
 * @param  item  - index of the item to test bits from
 * @param  tbits - bits to test in IACM 'item'
 * 
 * @return rbits - the results of the tested bits or all 1s if item is
 *                    invalid. 
 */
uint32_t iacm_testbits(IACM_ITEM_t item, uint32_t tbits)
{
    uint32_t rbits;
    
    int valid_cpy = iacm_verify();
    if(valid_cpy >= 0){
        rbits = iacm_layout[valid_cpy][item] & tbits;
    }else{
        rbits = -1;
    }
    return rbits;
}

uint32_t iacm_base_addr_get(void)
{
    return IACM_COPY0_BASE;
}

/**
 * @brief Sets a redundant iacm region to zero. Used for testing.
 * 
 * @param region - 0 to IACM_NUM_COPIES
 */
void iacm_corrupt_region(uint32_t region)
{
    if(region < IACM_NUM_COPIES){
        bzero(iacm_layout[region], IACM_CRC_LOCATION);
    }
}

uint32_t iacm_get_crpt_rr_msk(void)
{
    return iacm_found_crpt_rr_dbg_msk;
}
