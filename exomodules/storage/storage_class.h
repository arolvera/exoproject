//
// Created by marvin on 9/23/22.
//

#ifndef VORAGO_DEV_THRUSTER_CONTROL_COMMON_STORAGE_STORAGE_CLASS_H_
#define VORAGO_DEV_THRUSTER_CONTROL_COMMON_STORAGE_STORAGE_CLASS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct write_buf_t_ {
  uint8_t *page_buffer;      //buffer
  bool used;                 //is this buffer used
} write_buf_t;

typedef struct erase_buf_t {
  const uint32_t erase_page_min; //Number of pages that are erased when an erase cmd is sent
  const uint32_t erase_buffer_size; //Size of erase buffer
  uint8_t *erase_buffer;            //Used in an overlay write to store memory before erase
} erase_buf_t;

#define NUM_WRITERS 2

typedef struct fileops_t_ {
  void (*fops_open)(struct fileops_t_ *self, const char *file); //Init driver. x86 opens and mmap file
  void (*fops_close)(struct fileops_t_ *self);                  //deinit driver. x86 munmap
  int (*fops_write_page)(struct fileops_t_ *self, void *buf, int page); //Write a page into memory
  int (*fops_memcpy)(struct fileops_t_ *self, void *dst, void *src, int count); //Read from memroy
  int (*fops_erase_page)(struct fileops_t_ *self, int page_number);             //Erase page
  int (*fops_flash_lock)(struct fileops_t_ *self, int lock);                    //Lock flash
  int (*fops_flash_page_locker)(struct fileops_t_ *self, int page_number, int lock); //Lock page
  void (*fops_flash_write_protect)(struct fileops_t_ *self, int enable);             //Write protect device
  uint32_t flash_addr;  //start addr
  const uint32_t page_size;   //size of page. Not all device have the concept of a page
  const uint32_t num_regions; //Number of regions
  erase_buf_t erase_buf;
  write_buf_t write_buffers[NUM_WRITERS];
#ifdef __x86_64__
  uint8_t *memmap;
  char name[10]; //Used for debugging
#endif
} fileops_t;

typedef int (*WRITE_FUNC)(fileops_t*, void *data, int flash_offset);
int sm_overlay_write(fileops_t *fops, void *data, int flash_offset);
int sm_erase_write(fileops_t *fops, void *data, int flash_offset);

void f_init(void);
int f_write_page(fileops_t *self, void *buf, int oft);
int f_erase_page(fileops_t *self, int page_number);

#endif //VORAGO_DEV_THRUSTER_CONTROL_COMMON_STORAGE_STORAGE_CLASS_H_
