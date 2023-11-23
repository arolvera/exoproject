#include "storage_class.h"
#include "storage_memory_file_ops_driver.h"

#define SRAM_PAGE_SIZE 512
#define ERASE_PAGE_MINIMUM (16)
#define ERASE_SIZE (ERASE_PAGE_MINIMUM * SRAM_PAGE_SIZE)

static uint8_t erase_buffer[ERASE_SIZE];
static uint8_t page_buffer1[SRAM_PAGE_SIZE];
static uint8_t page_buffer2[SRAM_PAGE_SIZE];

fileops_t fops_fram = {
    .fops_open = fops_open_x86,
    .fops_close = fops_close_x86,
    .fops_write_page = fops_write_page_x86,
    .fops_memcpy = fops_memcpy_x86,
    .fops_erase_page = fops_erase_page_x86,
    .fops_flash_write_protect = fops_flash_write_protect_x86,
    .fops_flash_page_locker = fops_flash_page_locker_x86,

    .write_buffers = {
        {.page_buffer = page_buffer1, .used=false},
        {.page_buffer = page_buffer2, .used=false}},
    .flash_addr = 0,
    .page_size = SRAM_PAGE_SIZE,
    .num_regions = 1,
    .erase_buf.erase_page_min = ERASE_PAGE_MINIMUM,
    .erase_buf.erase_buffer_size = ERASE_PAGE_MINIMUM,
    .erase_buf.erase_buffer = erase_buffer,
    .name = "fram"
};
