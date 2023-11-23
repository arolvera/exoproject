#include <string.h>
#include "storage_memory_layout.h"
#include "storage_class.h"

#define SRAM_PAGE_SIZE 0x200
#define SRAM_ERASE_PAGE_MIN (1)
#define SRAM_ERASE_SIZE  (SRAM_ERASE_PAGE_MIN * SRAM_PAGE_SIZE)//arb size. Sram does not have pages
#define SRAM_NUM_REGIONS (4) //using rtt section

static volatile uint8_t
    ebi_sram_update_space[UPDATE_REGION_SIZE] __attribute__ ((section (".sys_ctrl_update_space")));

static void fops_open(fileops_t *self, const char *file);
static void fops_close(fileops_t *self);
static void fops_flash_write_protect(fileops_t *self, int enable);
static int fops_write_page(fileops_t *self, void *buf, int page);
static int fops_memcpy(fileops_t *self, void *dst, void *src, int count);
static int fops_erase_page(fileops_t *self, int page_number);
static int fops_flash_page_locker(fileops_t *self, int page_number, int lock);

static uint8_t erase_buffer[SRAM_ERASE_SIZE];
static uint8_t page_buffer1[SRAM_PAGE_SIZE];
static uint8_t page_buffer2[SRAM_PAGE_SIZE];

fileops_t fops_ebi_sram = {
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

    .flash_addr = (uint32_t)ebi_sram_update_space,
    .page_size = SRAM_PAGE_SIZE,
    .num_regions = 1,
    .erase_buf.erase_page_min = SRAM_ERASE_PAGE_MIN,
    .erase_buf.erase_buffer_size = SRAM_ERASE_SIZE,
    .erase_buf.erase_buffer = erase_buffer,
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
    (void)enable;
}

int fops_write_page(fileops_t *self, void *buf, int page)
{
    memcpy((void *)&ebi_sram_update_space[page * self->page_size], buf, SRAM_PAGE_SIZE);
    return 0;
}

int fops_memcpy(fileops_t *self, void *dst, void *src, int count)
{
    memcpy(dst, (void*)&ebi_sram_update_space[(uint32_t)src - self->flash_addr], count);
    return 0;
}

int fops_erase_page(fileops_t *self, int page_number)
{
    uint32_t arr_el = page_number * self->page_size;
    memset((void*)&ebi_sram_update_space[arr_el], 0, SRAM_ERASE_SIZE);
    return 0;
}

int fops_flash_page_locker(fileops_t *self, int page_number, int lock)
{
    (void)page_number;
    (void)lock;
    return 0;
}

