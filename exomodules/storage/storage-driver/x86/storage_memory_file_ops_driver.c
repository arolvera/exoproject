#include <memory.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "storage_memory_file_ops_driver.h"

#define X86_MAX_FILE_SIZE (0x200000) //Allow for 1MB each file. Upper limit.

void fops_open_x86(struct fileops_t_ *self, const char *file)
{
    char file_path[10] = "/tmp/";
    strncat(file_path, file, 1); //As of now all filenames are len of 1
    int fd = open(file_path, O_RDWR | O_SYNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1){
        printf("Error messge : %s\n", strerror(errno));
        exit(-1);
    }

    if(ftruncate(fd, X86_MAX_FILE_SIZE) == -1){
        printf("Error messge : %s\n", strerror(errno));
        exit(-1);
    }

    self->memmap = (uint8_t *)mmap(NULL, X86_MAX_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(self->memmap == (caddr_t)-1){
        printf("Error messge : %s\n", strerror(errno));
        exit(-1);
    }
    close(fd);
}

void fops_close_x86(struct fileops_t_ *self)
{
    munmap(self->memmap, X86_MAX_FILE_SIZE);
}

void fops_flash_write_protect_x86(fileops_t *fops, int enable)
{
    (void) enable;
}

int fops_write_page_x86(fileops_t *self, void *buf, int page)
{
    memcpy(&self->memmap[page * self->page_size], buf, self->page_size);
    return 0;
}

int fops_memcpy_x86(fileops_t *self, void *dst, void *src, int count)
{
    memcpy(dst, &self->memmap[(uint32_t)src - self->flash_addr], count);
    return 0;
}

int fops_erase_page_x86(fileops_t *self, int page_number)
{
    uint32_t arr_el = page_number * self->page_size;
    uint32_t mem_size = self->erase_buf.erase_page_min * self->page_size;
    memset(&self->memmap[arr_el], 0, mem_size);
    return 0;
}

int fops_flash_page_locker_x86(fileops_t *self, int page_number, int lock)
{
    (void)self;
    (void)page_number;
    (void)lock;
    //Maybe we could emulate page locking with memmap. but not for now.
    return 0;
}
