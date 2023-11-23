/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

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
#include <string.h>
#include <sys/stat.h>

#include "storage_memory.h"
#include "definitions.h"
#include "utils/stack.h"
#include "utils/macro_tools.h"
#include "trace/trace.h"
#include "storage/storage_class.h"
#include "osal/osal.h"
#include "component_map_def.h"
#include "fram/fram_va41630_cmn.h"
#include "flash/hal_flash.h"
#ifndef WATCHDOG_CLEAR
#define WATCHDOG_CLEAR()
#endif

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define NUM_FDS 11      // why 11 ????
static OSAL_STATIC_MUTEX_BUF erase_buf_mtx;
static OSAL_MUTEX_HANDLE_TYPE erase_mtx;

/* We are using acsii characters as a 'key' to the open function.
 * This allows us to take a string as an input to the open function for 
 * compatibility with standard open functions and allows us to quickly lookup
 * the accessor without doing a string compare and linear search.
 * We start 'A' (0x41) and increment from there.  That gives enough 'keys'
 * that are also printable characters.
 */
#define ACCESSOR_ID_BASE 'A'
#define ACCESSOR_ID_TO_INDEX(__X__)             \
({                                              \
    int retval = -1;                            \
    if(__X__ >= 'A' && __X__ <= 'Z') {          \
        retval = __X__  - 'A';                  \
    } else if(__X__ >= 'a' && __X__ <= 'z') {   \
        /* 26 letters in the alpabet (A-Z)      \
           lower case a-z is 26-51 */           \
        retval = 26 + ( __X__ - 'a');           \
    }                                           \
    retval;                                     \
})

//#define NUM_WRITERS 2
//write_buffers_t write_buffers[NUM_WRITERS];
static unsigned int active_writers = 0;

typedef struct {
    component_map_t *map;
    uint8_t* page_buffer;
    int bytes_cached;
    int index;
    int pos;
    int permissions;
    bool open;
    WRITE_FUNC write_func;
} fd_t;

static fd_t fds[NUM_FDS];
static stack_t sm_stack;

static int sm_flash_file_locker(const component_map_t *map, int lock)
{
    int err = 0;
    const component_t *comp = &map->component;
    fileops_t *fops = map->fops;

    uint32_t start_page = (comp->start_address - fops->flash_addr) / fops->page_size;
    uint32_t end_page   = (comp->end_address   - fops->flash_addr) / fops->page_size;
    
    TraceDbg(TrcMsgSm, "unlock sa:0x%x ea:0x%x s:0x%x e:0x%x",
            cm->start_address, cm->end_address, start_page, end_page,0,0);
    for(uint32_t i = start_page; i < end_page && !err; i += fops->num_regions) {        // ToDo: this was commented out. Why?
        err = fops->fops_flash_page_locker(fops, i, lock);
    }
    return err;
}

/**
 * Initialize the accessor.  Convert the path key into the table index
 * and open the file
 * @param path accessory key value
 * @param fd pointer to store the file descriptor
 * @param opts open options (O_WRONLY, O_RDONLY, etc.)
 * @return 0 on success, -1 otherwise
 */
static int sm_component_obj_init(const char* path, fd_t* fd, int opts)
{
    int err = 0;
    int comp_index = 0;
    
    /* Temp pointer - Do not do anything with FD, if anything fails we 
     * must push the fd back on the stack  */
    uint8_t *p = NULL;
    
    if(path == NULL || fd == 0) {
        err = -1;
    } else {
        comp_index = ACCESSOR_ID_TO_INDEX(path[0]);
        if(comp_index < 0 || comp_index >= (int)FILE_MAP_SIZE) {
            err = -1;
            TraceE3(TrcMsgErr3, "Invalid Key:0x%x, %d ", path[0], comp_index,0,0,0,0);
        }
    }
    if(!err) {
        /* Only one writer at a time. Check now so we do not have to undo a
         * partial open.
         */
        if((opts & O_WRONLY) && file_map[comp_index].open_for_write) {
            err = -1;
            TraceE3(TrcMsgErr3, "Component already open for write", 0,0,0,0,0,0);
        }
    }
    if(!err && opts & O_WRONLY) {
        /* Unlock this region */
        // TODO add this back in
        //sm_flash_file_locker(&file_map[comp_index], 0);
        for(int i = 0; i < NUM_WRITERS && p == NULL; i++){
            if(file_map[comp_index].fops->write_buffers[i].used != true){
                p = file_map[comp_index].fops->write_buffers[i].page_buffer;
                file_map[comp_index].fops->write_buffers[i].used = true;
            }
        }
        if(p == NULL){
            err = -1;
            TraceE3(TrcMsgErr3, "No page buffer available",0,0,0,0,0,0);
        }
    }
    if(!err) {
        /* preserve the index */
        int index = fd->index;
        memset(fd, 0, sizeof(fd_t));
        fd->index = index;
        fd->permissions = opts;
        fd->page_buffer = p; /*will be null if this is read */
        fd->open = true;
        fd->map = &file_map[comp_index];
        fd->write_func = file_map[comp_index].write_func;
        if(opts & O_WRONLY) {
            fd->map->open_for_write = true;
        }
        file_map[comp_index].fops->fops_open(file_map[comp_index].fops, path);
    }
    return err;
}

/**
 * Initialize the FD stack
 * @param stack_items number of items to initialize the stack
 * @return 
 */
static int sm_stack_init(int stack_items)
{
    int err = 0;
    if(stack_items < DEFAULT_STACK_SIZE){
        err = stack_init(&sm_stack);
        if (err == 0){
            for(int i = 0; i < stack_items; i++){
                stack_push(&sm_stack, &fds[i]);
                fds[i].index = i;
                fds[i].open = false;
            }
        }
    } else {
        err = -1;
    }
    return err;
}

/**
 * @brief Initialize storage memory module
 *
 * @param num_components to push onto sm_stack
 * 
 * @returns void
 */
void sm_init(void)
{
    fram_init();
    flash_init();
    sm_stack_init(NUM_FDS);
    OSAL_MUTEX_Create(&erase_mtx, &erase_buf_mtx, "storage_m");
}


/**
 * @brief Erase and write flash pages with settings in fops
 *
 * @param fops pointer to file operations for storage component
 * @param data pointer to data to write
 * @param flash_offset flash offset to write data to
 *
 * @returns 0 on success, !0 on failure
 */
int sm_erase_write(fileops_t *fops, void *data, int flash_offset)
{
    int err = 0;
    /* Page number is current offset (because we cache a page cache)
    * truncated on page boundary */
    int page_number = (int)(flash_offset - fops->flash_addr) / (int)fops->page_size;
    /* Erase before writing */
    TraceDbg(TrcMsgSm, "Write page:%x", page_number,0,0,0,0,0);

    OSAL_MUTEX_Lock(&erase_mtx, portMAX_DELAY);
    if((page_number % fops->erase_buf.erase_page_min) == 0) {
        err = fops->fops_erase_page(fops, page_number);
    }
    if(!err) {
        err = fops->fops_write_page(fops, data, page_number);
    }
    OSAL_MUTEX_Unlock(&erase_mtx);
    return err;
}




/**
 * @brief Function for reading memory from flash
 *
 *
 * @param fd similar to fd in POSIX filesystems. This is an integer that is unique 
 *        to whoever called sm_open().
 *
 * @param buf pointer to data structure in RAM used to store flash data
 * 
 * @param count number of bytes to read from flash
 *    
 * @returns The number of bytes read.  This function is only bound by the end of 
 *          flash in this implementation.
 */
ssize_t sm_read(int fd, void* buf, size_t cnt)
{
    int err = 0;
    fd_t *f = NULL;
    const component_t *c = NULL;
    
    int count = cnt;
    
    if(fd < 0 || fd >= NUM_FDS) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        c = &f->map->component;
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err && (f->permissions != O_RDONLY)) {
        err = -1;
        TraceE3(TrcMsgErr3, "File descriptor not open for read:%d perms:%x",
                fd, f->permissions,0,0,0,0);
    }
    if(!err) {
        /* The size of this flash region */
        int size = c->end_address - c->start_address;
        /* The new file pos (potentially) */
        int new_pos = f->pos + count;
        /* Get the real flash offset */
        uint32_t flash_offset = f->pos + c->start_address;
        
        /* If count pushes past end of flash area, truncate the read */
        if(new_pos > size) {
            count = size - f->pos;
        }
        //TraceDbg(TrcMsgSm, "fd:%d pos:%d cnt:%d fo:%x sz:%x np:%x",
        //        fd, f->pos, count, flash_offset, size, new_pos);
        /* Copy bytes to user buffer */
        err = f->map->fops->fops_memcpy(f->map->fops, buf, (uint32_t *)flash_offset, count);
        if(!err) {
            f->pos += count;
        }
    }
    /* return err, else bytes read */
    return err == 0 ? count : err;
}

/**
 * Central write routine.  This takes care of blocking the data into a page
 * size buffer and erasing the flash as needed for writes.
 * @note only sequential writes are supported.  When the write POS crosses
 * a 16 page boundary those 16 pages will be erased.
 * 
 * @param f pointer to file descriptor
 * @param buf pointer to store data
 * @param count number of bytes to read
 * @return number of bytes read on success or -1 on err
 */
static int sm_writer(fd_t *f, const void* buf, size_t count)
{
    const component_t *c = &f->map->component;
    fileops_t *fops = f->map->fops;
    
    int bytes_written = 0;
    int err = 0;
    /* The next file position will be here (potentially) */
    int next_pos = f->pos + count;
    /* Size of the flash area */
    int size = c->end_address - c->start_address;
    /* Check for write past end of flash and truncate */
    if( next_pos > size ) {
        count = size - f->pos;
    }
    /* The next bytes_cached count will be here (potentially) */
    uint32_t next_bytes_cached = f->bytes_cached + count;
    /* Current REAL offset in flash */
    uint32_t flash_offset = c->start_address + f->pos;
    /* bytes remaining in cache */
    uint32_t cache_space = fops->page_size - f->bytes_cached;
    /* new cache size - how much needs to be cached if it crosses a page */
    int new_cache_size = (int)(f->bytes_cached + count) % (int)fops->page_size;

    TraceDbg(TrcMsgSm, "nbc:%d np:%d size:%x oft:%x cs:%d ncs:%d",
            next_bytes_cached, next_pos, size, flash_offset,
            cache_space, new_cache_size);

    /* Check page boundary - cache data until it reaches or crosses a page */
    if(next_bytes_cached < fops->page_size){
        memcpy(&f->page_buffer[f->bytes_cached], buf, count);
        f->bytes_cached += count;
        f->pos += count;
        bytes_written = count;  /* we are done here */

    /* Else we need to write out a page and potentially cache data */
    } else {
        /* If next bytes cached hits a page boundary - neat, write it */
        if(next_bytes_cached == fops->page_size) {
            /* fill in the rest of our page cache */
            memcpy(&f->page_buffer[f->bytes_cached], buf, count);
            /* Write the page */
            err = f->write_func(fops, f->page_buffer, flash_offset);
            f->bytes_cached = 0;

        /* This write pushes over a boundary - Write and cache leftover */
        } else {
            /* move SOME data into the cache buffer */
            memcpy(&f->page_buffer[f->bytes_cached], buf, cache_space);
            /* Write the page */
            err = f->write_func(fops, (uint32_t*)f->page_buffer, flash_offset);

            /* copy remaining bytes in buf */
            memset(f->page_buffer, 0xff, fops->page_size);
            memcpy(f->page_buffer, buf + cache_space, new_cache_size);
            f->bytes_cached = new_cache_size;
        }
        /* This is the same is both of the above cases */
        f->pos += count;
        bytes_written = count;
        TraceDbg(TrcMsgSm, "pos:%d bw:%d cnt:%d", f->pos, bytes_written, count,0,0,0);
    }
    return err ? err:bytes_written;
}

/**
 * @brief Function for writing memory to flash
 *
 *
 * @param fd similar to fd in POSIX filesystems. This is an integer that is unique 
 *        to whoever called sm_open().
 *
 * @param buf pointer to data structure in RAM used to store data to be written to flash
 * 
 * @param count number of bytes to write to flash
 *    
 * @returns The number of bytes written.  This implementation does not allow any
 *          fd to write beyond its designate sector (see storage_memory_layout.h).
 */
ssize_t sm_write(int fd, const void* buf, size_t count)
{
    int err = 0;
    
    fd_t *f = NULL;
    
    if(fd < 0 || fd >= NUM_FDS) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err && !(f->permissions & O_WRONLY)) {
        err = -1;
        TraceE3(TrcMsgErr3, "File descriptor not open for write:%d perms:%x",
                fd, f->permissions,0,0,0,0);
    }
    if(!err) {
        err = sm_writer(f, buf, count);
    }
    return err;
}

/**
 * Flush any data that may be cached in the page buffer out to the EFC.
 * Any remaining room in the page buffer will be filled with F's and written out
 * @param fd open file descriptor
 * @return 0 on success, non-zero otherwise
 */
ssize_t sm_fflush(int fd)
{
    int err = 0;
    fd_t *f = NULL;
    fileops_t *fops;
    
    if(fd < 0 || fd >= (int)SIZEOF_ARRAY(fds)) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        fops = f->map->fops;
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err && !(f->permissions & O_WRONLY) ) {
        err = -1;
        TraceE3(TrcMsgErr3, "File descriptor not open for write:%d perms:%x",
                fd, f->permissions,0,0,0,0);
    }
    /* Only flush if some bytes are cached */
    if(!err && f->bytes_cached) {
        // Write the rest out with F's
        uint8_t data[fops->page_size];
        memset(data, 0xFF, fops->page_size);
        sm_writer(f, data, fops->page_size - f->bytes_cached);
    }
    return err;
}


/**
 * @brief Function for manipulating an fd's offset like a POSIX filesystem
 *
 *
 * @param fd similar to fd in POSIX filesystems. This is an integer that is unique 
 *        to whoever called sm_open().
 *
 * @param offset value to be used for fd.offset manipulation
 * 
 * @param whence integer for deciding how to evaluate fd.pos and offset param
 *    
 * @returns updated value of fd.pos.
 */
off_t sm_lseek(int fd, off_t offset, int whence)
{
    int err = 0;
    fd_t *f = NULL;
    
    if(fd < 0 || fd >= NUM_FDS) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err){
        if(whence == SEEK_CUR){
            f->pos += offset;
            err = f->pos;
        } else if(whence == SEEK_SET){
            f->pos = offset;
            err = f->pos;
        } else {
            err = -1;
            TraceE3(TrcMsgErr3, "Seek operation not supported, fd:%d whence:%d",
                    fd, whence,0,0,0,0);
        } 
        TraceDbg(TrcMsgSm, "err:%d fd:%d pos:%d oft:%d w:%d", err, fd,
                fds[fd].pos, offset, whence, 0);
    }
    return err;
}

static void sm_stat_info(struct stat* buf, component_map_t *map)
{
    buf->st_size = map->component.end_address - map->component.start_address;
    buf->st_blksize = map->fops->page_size;
    buf->st_blocks = buf->st_size / buf->st_blksize;
    /* ino_t is two bytes.  Use that to return the page offset in the EFC */
    buf->st_ino = (map->component.start_address -
            map->fops->flash_addr) / map->fops->page_size;
}

/**
 * Return  information  about  a  file, in the buffer pointed to by statbuf
 * @param fd open file descriptor
 * @param buf stat buffer to store file info
 * @return 0 on success, error otherwise
 */
int sm_fstat(int fd, struct stat* buf)
{
    int err = 0;
    fd_t *f = NULL;
    
    if(fd < 0 || fd >= NUM_FDS) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err && buf) {
        sm_stat_info(buf, f->map);
    }
    return err;
}
/**
 * Return  information  about  a  file, in the buffer pointed to by statbuf
 * @param filename accessor key
 * @param buf stat buffer to store file info
 * @return 0 on success, error otherwise
 */
int sm_stat(const char *filename, struct stat* buf)
{
    int err = 0;
    int comp_index = -1;
    component_map_t *map = NULL;
    
    comp_index = ACCESSOR_ID_TO_INDEX(filename[0]);
    if(comp_index < 0 || comp_index >= (int)FILE_MAP_SIZE) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid Key:%c", filename[0],0,0,0,0,0);
    } else {
        map = &file_map[comp_index];
    }
    if(!err && map && buf) {
        sm_stat_info(buf, map);
    }
    return err;
}

/**
 * @brief Function for assigning a unique value to a file descriptor
 * @param path used to find corresponding component_id
 * @param opts access permissions O_RDONLY, O_WRONLY, O_RDWR
 * @param ... not used in this implementation
 * @returns fd_i < 0 if no valid component_id found, > 0 if valid component_id found 
 */
int sm_open(const char* path, int opts, ...)
{
    if(opts & O_RDWR) {
        TraceE3(TrcMsgErr3, "RW access not supported", 0,0,0,0,0,0);
        return -1;
    }
    
    int err = 0;
    fd_t* fd = (fd_t *) stack_pop(&sm_stack);
    if(fd == NULL) {
        err = -1;
        TraceE3(TrcMsgErr3, "No fds available", 0,0,0,0,0,0);
    } else {
        err = sm_component_obj_init(path, fd, opts);
    }
    if(!err && opts & O_WRONLY) {
        active_writers++;
    }
    if(err && fd) {
        /* do not forget to push it back on the stack or we will run out */
        fd->open = false;
        stack_push(&sm_stack, fd);
    }
    return (err == 0) ? fd->index : err;
}

/**
 * @brief Zero out and push an fd back onto sm_stack
 *
 *
 * @param fd to zero and push
 *
 *    
 * @returns void 
 */
int sm_close(int fd)
{
    int err = 0;
    fd_t *f = NULL;
    fileops_t *fops;

    if(fd < 0 || fd >= NUM_FDS) {
        err = -1;
        TraceE3(TrcMsgErr3, "Invalid file descriptor:%d", fd, 0,0,0,0,0);
    }
    if(!err) {
        f = &fds[fd];
        fops = f->map->fops;
        if(f->open == false) {
            err = -1;
            TraceE3(TrcMsgErr3, "File descriptor not open:%d", fd, 0,0,0,0,0);
        }
    }
    if(!err){
        struct stat buf = {0};
        sm_fstat(fd, &buf);
        TraceDbg(TrcMsgSm, "fd:%d pos:%d sa:%x ea:%x", fd,
                fds[fd].pos,
                fds[fd].map->component.start_address,
                fds[fd].map->component.end_address, 0,0);
        if(f->permissions & O_WRONLY) {
            sm_fflush(fd);
            for(int i = 0; i < NUM_WRITERS; i++){
                if((uintptr_t)(f->page_buffer) == (uintptr_t)fops->write_buffers[i].page_buffer) {
                    fops->write_buffers[i].used = false;
                }
            }
            sm_flash_file_locker(f->map, 1);
            active_writers--;
            if(active_writers == 0) {
                fops->fops_flash_write_protect(fops, 1);
            }
            f->map->open_for_write = false;
        }
        f->open = false;
        stack_push(&sm_stack, f);
        fops->fops_close(fops);
    } else {
        err = -1;
    }
    return err;
}
