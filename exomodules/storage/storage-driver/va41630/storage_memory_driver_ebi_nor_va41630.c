#include <string.h>
#include "definitions.h"
#include "trace/trace.h"
#include "storage_class.h"

#include "flash/hal_flash.h"
#include "atomic.h"

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

//JLK when driver is written grab page size from driver
#define NOR_SIZE 0x1000000

/* 2 bytes is the minimum we can write to this nor chip */
#define NOR_PAGE_SIZE (2)
#define NOR_SECTOR_SIZE 0x10000
/* NOTE: this is not the actual nor start address, but it makes
 * the over-abstracted math work better */
#define NOR_START_ADDRESS 0x60040000
#define NOR_PAGE_PER_BLK (16)
#define ERASE_SIZE (NOR_PAGE_PER_BLK * NOR_PAGE_SIZE)
static uint8_t erase_buffer[ERASE_SIZE];

static void fops_open(fileops_t *self, const char *file);
static void fops_close(fileops_t *self);
static void fops_flash_write_protect(fileops_t *self, int enable);
static int fops_write_page(fileops_t *self, void *buf, int page);
static int fops_memcpy(fileops_t *self, void *dst, void *src, int count);
static int fops_erase_page(fileops_t *self, int page_number);
static int fops_flash_page_locker(fileops_t *self, int page_number, int lock);

static uint8_t page_buffer1[NOR_PAGE_SIZE];
static uint8_t page_buffer2[NOR_PAGE_SIZE];

fileops_t fops_ebi_nor = {
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

    .flash_addr = NOR_START_ADDRESS,
    .page_size = NOR_PAGE_SIZE,
    .num_regions = 16, // NOR chip has 16 lockable sectors
    .erase_buf.erase_page_min = NOR_SECTOR_SIZE,
    .erase_buf.erase_buffer_size = ERASE_SIZE,
    .erase_buf.erase_buffer = erase_buffer,
};

void f_init(void)
{
    flash_init();
}

void fops_open(fileops_t *self, const char *file)
{
    flash_init();
    (void)self;
    (void)file;
}

void fops_close(fileops_t *self)
{
    (void)self;
}

void fops_flash_write_protect(fileops_t *self, int enable)
{
    (void)self;
    (void)enable;
}


int fops_write_page(fileops_t *self, void *buf, int oft)
{
    int err = 0;

    /* Copy to local stack buffer so we're not accessing static data with SRAM
     *   at the same time as accessing NOR flash, thus confusing the EBI bus */
    uint16_t page_buf = *((uint16_t*)buf);

    /* If offset is less than the start of flash or greater than the end of
     * flash minus page - stop, log error, do not hard fault! */
    if(oft < 0 || oft >= (NOR_SIZE - NOR_PAGE_SIZE)) {
        TraceE3(TrcMsgErr3, "Offset outside of flash range:%x", oft, 0, 0, 0, 0, 0);
        err = -1;
    }
    if(!err) {
        ATOMIC_ENTER_CRITICAL();
        flash_single_write(oft, page_buf);
        // = flash_single_read(oft);
        ATOMIC_EXIT_CRITICAL();
    }

    return err;
}

int fops_memcpy(fileops_t *self, void *dst, void *src, int count)
{
    int err = 0;

    TraceDbg(TrcMsgSm, "dst:%p src:%p cnt:%x", dst,src,count,0,0,0);

    if((uint32_t)src < NOR_START_ADDRESS ||
        (uint32_t)(src + count) >= (NOR_START_ADDRESS + (NOR_SIZE - NOR_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Outside of flash range. dst:%p count:%d", src, count, 0, 0, 0, 0);
        err = -1;
    }

    if(!err) {
        /* Nor flash doesn't use the ebi bus, and is 16 bit, so we have to play some games */
        uint32_t nor_addr_actual = ((uint32_t)src - NOR_START_ADDRESS) / sizeof(uint16_t);
        for(unsigned int i = 0; i < count / sizeof(uint16_t); i++){
            ((uint16_t*)dst)[i] = flash_single_read(nor_addr_actual + i);
        }
    }

    return err;
}

int fops_erase_page(fileops_t *self, int page_number)
{
    ATOMIC_ENTER_CRITICAL();
    /* Mask off upper 9 bits to extract sector address to erase before writing */
    volatile int sector = (page_number & 0x1FF0000) >> 16;
    flash_sector_erase(sector);

    ATOMIC_EXIT_CRITICAL();
    return 0;
}

int fops_flash_page_locker(fileops_t *self, int page_number, int lock)
{
    return 0;
}
