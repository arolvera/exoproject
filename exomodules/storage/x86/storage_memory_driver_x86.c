#include <string.h>
#include <sys/stat.h>
#include "definitions.h" /* defines EFC_ APIs */
#include "utils/stack.h"
#include "utils/macro_tools.h"
#include "trace/trace.h"
#include "osal/osal_freertos.h"
#include "storage/storage_class.h"
#include "storage/storage_memory_interface.h"
#include "storage/storage_memory_layout.h"
#include "storage/component_keys.h"
#include "magic_numbers.h"


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
uint8_t erase_buffer[ERASE_SIZE];

static void sm_flash_write_protect(int enable);
static int sm_write_page(void *buf, int oft);
static int sm_memcpy(void *dst, void *src, int count);
static int sm_erase_page(int page_number);
static int sm_flash_page_locker(int page_number, int lock);

static uint8_t page_buffer1[IFLASH_PAGE_SIZE];
static uint8_t page_buffer2[IFLASH_PAGE_SIZE];

fileops_t fops = {
    .sm_write_page = sm_write_page,
    .sm_memcpy = sm_memcpy,
    .sm_erase_page = sm_erase_page,
//    .sm_lock_bits_get = sm_lock_bits_get,
    .write_buffers = {{.page_buffer = page_buffer1, .page_buffer_size=IFLASH_PAGE_SIZE, .used=false},
        {.page_buffer = page_buffer2, .page_buffer_size=IFLASH_PAGE_SIZE, .used=false}},
    .sm_flash_page_locker = sm_flash_page_locker,
    .sm_flash_write_protect = sm_flash_write_protect,

    .flash_addr = FLASH_START_ADDRESS,
    .page_size = IFLASH_PAGE_SIZE,
    .num_regions = IFLASH_NUM_REGIONS,
    .erase_page_min = ERASE_PAGE_MINIMUM,
    .erase_buffer_size = ERASE_PAGE_MINIMUM,
    .erase_buffer = erase_buffer,
};

static void sm_flash_write_protect(int enable)
{
    return;
}

#define IFLASH_SIZE 0x00200000
static int sm_write_page(void *buf, int oft)
{
    int err = 0;
    uint32_t *data = (uint32_t *)buf;

    /* If offset is less than the start of flash or greater than the end of
     * flash minus page - stop, log error, do not hard fault! */
    if(oft < FLASH_START_ADDRESS || oft >= (FLASH_START_ADDRESS + (IFLASH_SIZE - IFLASH_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Offset outside of flash range:%x", oft, 0, 0, 0, 0, 0);
        err = -1;
    }
    if(!err) {
        uint16_t page_number;

        /*Calculate the Page number to be passed for FARG register*/
        page_number = (oft - FLASH_START_ADDRESS) / IFLASH_PAGE_SIZE;

        TraceDbg(TrcMsgSm, "oft:%x page:%x", oft, page_number, 0, 0, 0, 0);
        for(uint32_t i = 0; i < IFLASH_PAGE_SIZE; i += 4) {
            ebi_write(
                *((uint32_t *)(FLASH_START_ADDRESS + (page_number * IFLASH_PAGE_SIZE) + i)),
                *((data++)));
        }

        __DSB();
        __ISB();

//        /* Issue the FLASH write operation*/
//        EFC_REGS->EEFC_FCR = (EEFC_FCR_FCMD_WP | EEFC_FCR_FARG(page_number) | EEFC_FCR_FKEY_PASSWD);
//
//        /* @fixme needs a timeout - can't wait forever */
//        while(EFC_IsBusy());
//
//        err = EFC_REGS->EEFC_FSR;
//        /* @fixme Do not look for ECC errors here - but this clears them, so... */
//        err &= ~EFC_ECC_ERROR;
//        if(err == EFC_ERROR_NONE) {
//            err = 0;
//        }else {
//            TraceE2(TrcMsgErr2, "EFC error:%x offset:%p", err, oft, 0, 0, 0, 0);
//            err = -1;
//        }
    }
    return err;
}
static int sm_memcpy(void *dst, void *src, int count)
{
    int err = 0;

//TraceDbg(TrcMsgSm, "dst:%p src:%p cnt:%x", dst,src,count,0,0,0);

    if((uint32_t)src < FLASH_START_ADDRESS ||
        (uint32_t)(src + count) >= (FLASH_START_ADDRESS + (IFLASH_SIZE - IFLASH_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Outside of flash range. dst:%p count:%d", src, count, 0, 0, 0, 0);
        err = -1;
    }

    if(!err) {
        memcpy(dst, src, count);
    }

//    err = EFC_ErrorGet();
//    if(err & EFC_ECC_ERROR) {
//        TraceE2(TrcMsgErr2, "EFC ECC error:%x offset:%p", err, src, 0, 0, 0, 0);
///* @fixme Do not look for ECC errors on erase */
//        err &= ~EFC_ECC_ERROR;
//    }
//
//    if(err == EFC_ERROR_NONE) {
//        err = 0;
//    }else {
//        TraceE2(TrcMsgErr2, "EFC error:%x offset:%p", err, dst, 0, 0, 0, 0);
//        err = -1;
//
//    }
    return err;
}

static int sm_erase_page(int page_number)
{
    return 0;
}
static int sm_flash_page_locker(int page_number, int lock)
{
    return 0;
}
