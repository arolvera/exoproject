/* Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

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
#include "definitions.h" /* defines EFC_ APIs */
#include "magic_numbers.h"
#include "osal/osal.h"
#include "storage/component_keys.h"
#include "storage/memory-component/halo6/storage/storage_memory_layout.h"
#include "storage/storage_class.h"
#include "storage/storage_memory_interface.h"
#include "utilities/macro_tools.h"
#include "utilities/stack.h"
#include "utilities/trace/trace.h"
#include <string.h>
#include <sys/stat.h>

#ifndef WATCHDOG_CLEAR
#define WATCHDOG_CLEAR()
#endif

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

#define ERASE_PAGE_MINIMUM (16)
#define ERASE_SIZE (ERASE_PAGE_MINIMUM * IFLASH_PAGE_SIZE)
static uint8_t erase_buffer[ERASE_SIZE];

static void fops_open(fileops_t *self, const char *file);
static void fops_close(fileops_t *self);
static void fops_flash_write_protect(fileops_t *self, int enable);
static int fops_write_page(fileops_t *self, void *buf, int oft);
static int fops_memcpy(fileops_t *self, void *dst, void *src, int count);
static int fops_erase_page(fileops_t *self, int page_number);
static int fops_flash_page_locker(fileops_t *self, int page_number, int lock);

static uint8_t page_buffer1[IFLASH_PAGE_SIZE];
static uint8_t page_buffer2[IFLASH_PAGE_SIZE];

fileops_t fops_eefc = {
    .fops_open = fops_open,
    .fops_close = fops_close,
    .fops_write_page = fops_write_page,
    .fops_memcpy = fops_memcpy,
    .fops_erase_page = fops_erase_page,
    .write_buffers = {
        {.page_buffer = page_buffer1, .used=false},
        {.page_buffer = page_buffer2, .used=false}},
    .fops_flash_page_locker = fops_flash_page_locker,
    .fops_flash_write_protect = fops_flash_write_protect,

    .flash_addr = IFLASH_ADDR,
    .page_size = IFLASH_PAGE_SIZE,
    .num_regions = EFC_LOCK_REGIONS,
    .erase_page_min = ERASE_PAGE_MINIMUM,
    .erase_buffer_size = ERASE_SIZE,
    .erase_buffer = erase_buffer,
};

void fops_open(fileops_t *self, const char *file)
{
    (void)self;
    (void)file;
}

void fops_close(fileops_t *self)
{
    (void)self;
}

void fops_flash_write_protect(fileops_t *self, int enable)
{
    int ena = enable ? 1 : 0;
    EFC_REGS->EEFC_WPMR = (EEFC_WPMR_WPEN(ena) | EEFC_WPMR_WPKEY_PASSWD);
    while(EFC_IsBusy());
}

static int sm_flash_page_locker(int page_number, int lock)
{
    int err = 0;

    WATCHDOG_CLEAR();  /* This can take awhile */

    uint32_t sc = EEFC_FCR_FCMD_SLB; /* set/clear - set (lock) by default */
    EFC_ERROR efc_error = EFC_ERROR_NONE;

    TraceDbg(TrcMsgSm, "l:%d pn:%x", lock, page_number, 0, 0, 0, 0);

    if(!lock) {
        sm_flash_write_protect(0);
        sc = EEFC_FCR_FCMD_CLB;
    };

    EFC_REGS->EEFC_FCR = (sc | EEFC_FCR_FARG(page_number) | EEFC_FCR_FKEY_PASSWD);
    while(EFC_IsBusy());

    efc_error = EFC_ErrorGet();
    /* Do not look for ECC errors on erase */
    efc_error &= ~EFC_ECC_ERROR;
    if(efc_error != EFC_ERROR_NONE) {
        err = __LINE__;
        TraceE3(TrcMsgErr3, "EFC error on unlock page:0x%x pn:0x%x",
                efc_error, page_number, 0, 0, 0, 0);
    }
    return err;
}

int fops_write_page(fileops_t *self, void *buf, int oft)
{
    int err = 0;
    uint32_t *data = (uint32_t *)buf;

    /* If offset is less than the start of flash or greater than the end of
     * flash minus page - stop, log error, do not hard fault! */
    if(oft < IFLASH_ADDR || oft >= (IFLASH_ADDR + (IFLASH_SIZE - IFLASH_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Offset outside of flash range:%x", oft, 0, 0, 0, 0, 0);
        err = -1;
    }
    if(!err) {
        uint16_t page_number;

        /*Calculate the Page number to be passed for FARG register*/
        page_number = (oft - IFLASH_ADDR) / IFLASH_PAGE_SIZE;

        TraceDbg(TrcMsgSm, "oft:%x page:%x", oft, page_number, 0, 0, 0, 0);
        for(uint32_t i = 0; i < IFLASH_PAGE_SIZE; i += 4) {
            *((uint32_t *)(IFLASH_ADDR + (page_number * IFLASH_PAGE_SIZE) + i)) = *((data++));
        }

        __DSB();
        __ISB();

        /* Issue the FLASH write operation*/
        EFC_REGS->EEFC_FCR = (EEFC_FCR_FCMD_WP | EEFC_FCR_FARG(page_number) | EEFC_FCR_FKEY_PASSWD);

        /* @fixme needs a timeout - can't wait forever */
        while(EFC_IsBusy());

        err = EFC_REGS->EEFC_FSR;
        /* @fixme Do not look for ECC errors here - but this clears them, so... */
        err &= ~EFC_ECC_ERROR;
        if(err == EFC_ERROR_NONE) {
            err = 0;
        }else {
            TraceE2(TrcMsgErr2, "EFC error:%x offset:%p", err, oft, 0, 0, 0, 0);
            err = -1;
        }
    }
    return err;
}

/**
 * Central function to do the reads (memcpy) from the EFC module.  This will
 * range check the source address to make sure it is within the EFC memory area
 * @param dst pointer to copy data to
 * @param src pointer within the EFC memory region to copy data from
 * @param count how many bytes to copy
 * @return 0 on success, none zero otherwise
 * @fixme during testing on the Xplained board occasional ECC erros would happen
 * That err is being ignored for now.  This needs to be fixed when memory
 * scrubbing is enabled.
 */
static int sm_memcpy(void *dst, void *src, int count)
{
    EFC_ERROR err = 0;

    //TraceDbg(TrcMsgSm, "dst:%p src:%p cnt:%x", dst,src,count,0,0,0);

    if((uint32_t)src < IFLASH_ADDR ||
        (uint32_t)(src + count) >= (IFLASH_ADDR + (IFLASH_SIZE - IFLASH_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Outside of flash range. dst:%p count:%d", src, count, 0, 0, 0, 0);
        err = -1;
    }

    memcpy(dst, src, count);
    while(EFC_IsBusy());

    err = EFC_ErrorGet();
    if(err & EFC_ECC_ERROR) {
        TraceE2(TrcMsgErr2, "EFC ECC error:%x offset:%p", err, src, 0, 0, 0, 0);
        /* @fixme Do not look for ECC errors on erase */
        err &= ~EFC_ECC_ERROR;
    }

    if(err == EFC_ERROR_NONE) {
        err = 0;
    }else {
        TraceE2(TrcMsgErr2, "EFC error:%x offset:%p", err, dst, 0, 0, 0, 0);
        err = -1;
    }
    return err;
}

/**
 * Erase 16 Pages starting at the specified page boundary
 * @param page_number first page in the 16 pages to be erased.
 * @return 0 on success or -1 on failure.
 */
int fops_memcpy(fileops_t *self, void *dst, void *src, int count)
{
    int err = 0;
    EFC_ERROR efc_error = EFC_ERROR_NONE;
    TraceDbg(TrcMsgSm, "pn:%x", page_number, 0, 0, 0, 0, 0);
    // erase 16 pages
    EFC_REGS->EEFC_FCR =
        (EEFC_FCR_FCMD_EPA |
            EEFC_FCR_FARG(page_number | 0x2) | EEFC_FCR_FKEY_PASSWD);
    while(EFC_IsBusy());
    efc_error = EFC_ErrorGet();
    /* Do not look for ECC errors on erase */
    efc_error &= ~EFC_ECC_ERROR;
    if(efc_error != EFC_ERROR_NONE) {
        err = -1;
        TraceE3(TrcMsgErr3, "EFC error on erase:%x", efc_error, 0, 0, 0, 0, 0);
    }
    return err;
}

/**
 * @param lbs - pointer from the caller to the lock bit storage
 *
 */
void sm_lock_bits_get(efc_lock_bits_t *lock_bits)
{
    uint32_t *lbs = (uint32_t *)lock_bits;

    EFC_REGS->EEFC_FCR = (EEFC_FCR_FCMD_GLB | EEFC_FCR_FKEY_PASSWD);

    for(int i = 0; i < EFC_LOCK_STATUS_WORDS; i++) {
        lbs[i] = EFC_REGS->EEFC_FRR;
        TraceDbg(TrcMsgSm, "lbs[%d]:0x%08x", i, lbs[i], 0, 0, 0, 0);
    }
}

int sm_flash_lock(int lock)
{
    int err = 0;
    int i;
    efc_lock_bits_t lock_bits;

    uint32_t expected_lock_status = lock ? ~0:0;

    for(int  i=0; i < EFC_LOCK_REGIONS; i++) {
        int page_number = (i * EFC_LOCK_REGION_SIZE) / IFLASH_PAGE_SIZE;
        sm_flash_page_locker(page_number, lock);
    }
    if(lock) { /* Set Write Protect Pin */
        sm_flash_write_protect(1);
    }
    sm_lock_bits_get(&lock_bits);
    for(i = 0; i < EFC_LOCK_STATUS_WORDS && !err; i++) {
        if(lock_bits.lock_bits[i] != expected_lock_status) {
            err = __LINE__;
        }
    }
    WATCHDOG_CLEAR();
    return err;
}
