#include <string.h>
#include "trace/trace.h"
#include "fram/fram_va41630_cmn.h"
#include "storage_class.h"
#include "cmsis/cmsis_compiler.h"
#include "storage_memory_layout.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

#define FRAM_ERASE_PAGE_MINIMUM (16)
#define ERASE_SIZE (FRAM_ERASE_PAGE_MINIMUM * FRAM_PAGE_SIZE)

static void fops_open(fileops_t *self, const char *file);
static void fops_close(fileops_t *self);
static void fops_flash_write_protect(fileops_t *self, int enable);
static int fops_write_page(fileops_t *self, void *buf, int page);
static int fops_memcpy(fileops_t *self, void *dst, void *src, int count);
static int fops_erase_page(fileops_t *self, int page_number);
static int fops_flash_page_locker(fileops_t *self, int page_number, int lock);

static uint8_t erase_buffer[ERASE_SIZE];
static uint8_t page_buffer1[FRAM_PAGE_SIZE];
static uint8_t page_buffer2[FRAM_PAGE_SIZE];

fileops_t fops_fram = {
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

    .flash_addr = FRAM_START_ADDR,
    .page_size = FRAM_PAGE_SIZE,
    .num_regions = 1,
    .erase_buf.erase_page_min = FRAM_ERASE_PAGE_MINIMUM,
    .erase_buf.erase_buffer_size = ERASE_SIZE,
    .erase_buf.erase_buffer = erase_buffer,
};

void fops_open(fileops_t *self, const char *file) {
    (void)self;
    (void)file;
}

void fops_close(fileops_t *self)
{
    (void)self;
}

void fops_flash_write_protect(fileops_t *self, int enable)
{
    if(!enable) {
        fram_sleep();
    }
}


int fops_write_page(fileops_t *self, void *buf, int page)
{
    int err = 0;
    long unsigned int offst = page * FRAM_PAGE_SIZE;

    /* If offset is less than the start of flash or greater than the end of
     * flash minus page - stop, log error, do not hard fault! */
    if((int)offst < FRAM_START_ADDR || offst >= (FRAM_START_ADDR + (FRAM_SIZE - FRAM_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Offset outside of flash range:%x", page, 0, 0, 0, 0, 0);
        err = -1;
    }
    if(!err) {

        TraceDbg(TrcMsgSm, "oft:%x pg:%x", offst, page, 0, 0, 0, 0);
        fram_write(offst,
                   (uint8_t *)buf,
                    FRAM_PAGE_SIZE);
        __DSB();
        __ISB();
    }
    return err;
}

int fops_memcpy(fileops_t *self, void *dst, void *src, int count)
{
    int err = 0;

    TraceDbg(TrcMsgSm, "dst:%p src:%p cnt:%x", dst, src, count, 0, 0, 0);

    if((int32_t)src < FRAM_START_ADDR || (uint32_t)(src + count) >= (FRAM_START_ADDR + (FRAM_START_ADDR - FRAM_PAGE_SIZE))) {
        TraceE3(TrcMsgErr3, "Outside of flash range. dst:%p count:%d", src, count, 0, 0, 0, 0);
        err = -1;
    }

    if(!err) {
        fram_read((uint32_t)src, dst, count);
    }

    return err;
}

int fops_erase_page(fileops_t *self, int page_number)
{
    uint32_t ofst = FRAM_PAGE_SIZE * page_number;
    uint8_t buf[FRAM_PAGE_SIZE] = {0};
    fram_write(ofst, buf, FRAM_PAGE_SIZE);
    return 0;
}

int fops_flash_page_locker(fileops_t *self, int page_number, int lock)
{
    (void)page_number;
    (void)lock;
    return 0;
}

