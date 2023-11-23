#ifndef HARDWARECONTROL_HALO12_X86_XENON_SILVER_BOEING_EXOMODULES_STORAGE_STORAGE_DRIVER_X86_STORAGE_MEMORY_FILE_OPS_DRIVER_H_
#define HARDWARECONTROL_HALO12_X86_XENON_SILVER_BOEING_EXOMODULES_STORAGE_STORAGE_DRIVER_X86_STORAGE_MEMORY_FILE_OPS_DRIVER_H_

#include "storage_class.h"
void fops_open_x86(struct fileops_t_ *self, const char *file);
void fops_close_x86(struct fileops_t_ *self);
void fops_flash_write_protect_x86(fileops_t *fops, int enable);
int fops_write_page_x86(fileops_t *self, void *buf, int oft);
int fops_memcpy_x86(fileops_t *self, void *dst, void *src, int count);
int fops_erase_page_x86(fileops_t *self, int page_number);
int fops_flash_page_locker_x86(fileops_t *self, int page_number, int lock);

#endif //HARDWARECONTROL_HALO12_X86_XENON_SILVER_BOEING_EXOMODULES_STORAGE_STORAGE_DRIVER_X86_STORAGE_MEMORY_FILE_OPS_DRIVER_H_
