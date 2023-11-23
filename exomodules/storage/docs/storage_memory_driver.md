
## Storage Memory
storage_memory.c uses defined fileops_t for that memory section to interface with memory device.

![Storage Memory Driver](Storage_Memory.svg)

## File Map
Each project gets a file map that describes how different component's are stored in memory. 
- File Name - One character that is used as a key to find memory component
- File Size & Addr - Size of File and start address of component
- Write Func Type - Overlay write (Used when you dont have to erase a sector before write). Erase write (erase sector before write)
- Storage Memory Driver - Driver to interface with storage device

### Example of filemap
```
// Each driver is externed in filemap. A definition for that driver is given in 
extern fileops_t fops_ebi_nor;

//////////////////////////////////// ToDo: this has changed
component_map_t  file_map[] = {
    {UPDATE_IMAGE, {UPDATE_IMAGE_FLASH_START, UPDATE_IMAGE_FLASH_START + MASTER_IMAGE_AREA}, false, sm_erase_write, &fops_ebi_nor},
    .
    .
    .
    }
```

## Storage Memory Driver (fileops_t)
Common interface to interface with storage memory device. Each processor specific storage driver should have a x86 equivalent. All x86 drivers work the same way.

```
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
  int (*fops_write_page)(struct fileops_t_ *self, void *buf, int oft); //Write a page into memory
  int (*fops_memcpy)(struct fileops_t_ *self, void *dst, void *src, int count); //Read from memroy
  int (*fops_erase_page)(struct fileops_t_ *self, int page_number);             //Erase page
  int (*fops_flash_lock)(struct fileops_t_ *self, int lock);                    //Lock flash
  int (*fops_flash_page_locker)(struct fileops_t_ *self, int page_number, int lock); //Lock page
  void (*fops_flash_write_protect)(struct fileops_t_ *self, int enable);             //Write protect device

  const uint32_t flash_addr;  //start addr
  const uint32_t page_size;   //size of page. Not all device have the concept of a page
  const uint32_t num_regions; //Number of regions
  erase_buf_t erase_buf;
  write_buf_t write_buffers[NUM_WRITERS];
#ifdef __x86_64__
  uint8_t *memmap;
#endif
} fileops_t;
```

## NOTES:
Clients can utilize storage module and update command but make sure errors are not set or init errors. Only alerts can be used. Any error that changes canopen state cannot be used.