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

#include <unistd.h>
#include "driver/co_nvm_VA41630.h"

#define CO_NVM_BASE_ADDR    0x5F0000UL /* End of EFC flash */
#define CO_NVM_SIZE         0x010000UL /* 64K */



static void     DrvNvmInit  (void);
static uint32_t DrvNvmRead  (uint32_t start, uint8_t *buffer, uint32_t size);
static uint32_t DrvNvmWrite (uint32_t start, uint8_t *buffer, uint32_t size);



/* TODO: rename the variable to match the naming convention: 
 *   <device>NvmDriver
 */
const CO_IF_NVM_DRV ATSAMV71NvmDriver = {
    DrvNvmInit,
    DrvNvmRead,
    DrvNvmWrite
};



static void DrvNvmInit(void)
{
    
}

static uint32_t DrvNvmRead(uint32_t start, uint8_t *buffer, uint32_t size)
{
    int err = -1;
#if 0 // @fixme efc_ functions depracated
    int fd = efc_fd_open(ACCESSOR_ID_CAN_OPEN, 0);
    if(fd >= 0) {
        efc_flash_lseek(fd, start, SEEK_SET);
        err = efc_flash_read(fd, buffer, size);
    }
    efc_fd_close(fd);
#endif
    return err;
}

static uint32_t DrvNvmWrite(uint32_t start, uint8_t *buffer, uint32_t size)
{
    int err = -1;
#if 0 // @fixme efc_ functions depracated
    int fd = efc_fd_open(ACCESSOR_ID_CAN_OPEN, 0);
    if(fd >= 0) {
        efc_flash_lseek(fd, start, SEEK_SET);
        err = efc_flash_write(fd, buffer, size);
    }
    efc_fd_close(fd);
#endif
    return err;
}
