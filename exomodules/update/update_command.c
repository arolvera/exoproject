/**
 * @file    update_command.c
 *
 * @brief   This is the implementation for the command and control to update the system.
 * It has prepare (open the file), program (write a buffer), verify, and install
 * the image.
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#include "update_command.h"
#include "definitions.h"
#include "error/error_handler.h"
#include "iacm/iacm.h"
#include "icm/tc_icm.h"// Disable ICM before doing update
#include "storage/storage_memory_layout.h"
#include "storage_helper.h"
#include "thruster_control.h"
#include "trace/trace.h"  // Humanly readable trace messages
#include "update_helper.h"// update helper functions
#include <stdint.h>

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

typedef enum{
    UPDATE_COMMAND_SUBMODULE
}update_submodules_t;

#define MODULE_NUM MODULE_NUM_UPDATE_COMMAND
#define SUBMODULE_NUM UPDATE_COMMAND_SUBMODULE

#define DETAIL_LOG_LENGTH LOG_LENGTH_16

typedef struct update_error_detail{
    base_error_detail_t b_d;
    update_error_specific_detail_t update_error_specific_detail;
} update_error_detail_t;

static update_error_detail_t error_detail[DETAIL_LOG_LENGTH];

/* Update file FD */
static int update_fd = -1;
/* Current update write position */
static uint32_t pos = 0;
/* Size of the current update transfer */
static uint32_t transfer_size = 0;



static void* uc_error_detail_strategy(void* arg)
{
    if(arg != NULL){
        EH_LOG_ERROR(update_error_specific_detail, arg);
    }
    
    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}

static void uc_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, uc_error_detail_strategy, LOG_LENGTH_16, error_detail);
    
    fh_fault_handlers_register(ERROR_CODE_BL_INSTALL_ERROR, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_INSTALL_ERROR, FH_ALERT);
}

void uc_init(void) 
{
    uc_error_handler_init();
}

int uc_prepare(int size)
{
    int err = 0;
    if(update_fd >= 0 || transfer_size > 0) {
        err = __LINE__;
        TraceE3(TrcMsgUpdate, "Transfer already in progress", 0,0,0,0,0,0);
    }
    if(size > UPDATE_REGION_SIZE) {
        err = __LINE__;
        TraceE3(TrcMsgUpdate, "Transfer size too big r:0x%x s:0x%x",
                size, UPDATE_REGION_SIZE,0,0,0,0);
    }
    if(!err) {
        err = tc_icm_lock();
    }
    if(!err) {
        pos = 0;
        err = storage_memory_open((const char *)UPDATE_IMAGE, O_WRONLY);
        if(err >= 0) {
            /* Save the transfer size and file descriptor in static memory */
            transfer_size = size;
            update_fd = err;
            err = 0;
        } else {
            transfer_size = -1;
            update_fd = -1;
            tc_icm_unlock();
        }
    }
    return err;
}

int uc_upload(void* buf, uint32_t size)
{
    int err = __LINE__;
    if (size > 0 && update_fd >= 0) {
        err = storage_memory_write(update_fd, buf, size);   // returns bytes written
        if (err == (int)size) {
            pos += size;
            err = 0;
        } else {
            TraceE3(TrcMsgErr3, "Write failed. fd:%d size:%d err:%d", update_fd, size, err, 0, 0, 0);
            err = __LINE__;
        }
        /* If there is an error or we are done */
        if (pos >= transfer_size || err) {
            storage_memory_close(update_fd);
            update_fd = -1;
            pos = 0;
        }
    } else if(pos > 0 && size == 0) {
        /* we have been aborted, clean up */
        if(update_fd > -1) {
            storage_memory_close(update_fd);
        }
        update_fd = -1;
        pos = 0;
        transfer_size = 0;
        TraceInfo(TrcMsgUpdate, "Update aborted", 0,0,0,0,0,0);
    }
    if(update_fd < 0) {
        /* one way or the other, we are done, release the lock */
        tc_icm_unlock();
    }

    return err;
}

int uc_verify(void)
{
    int err = 0;

    err = uh_update_crc_inspect();

    return err;
}

int uc_install(void)
{
    int err = 0;

    // First make sure there really is valid image in SRAM
    err = uh_update_crc_inspect();

    if(!err) {
        TraceE3(TrcMsgErr3, "Install ecpk image, err = %d\n", err, 0,0,0,0,0);

        err = uh_install_ecpk_image();
    }


    if(err) {

        // Lets upload the stale image from NOR flash - this checks crc's after copy
        err = uh_upload_stale_update();
    } else {
        // We have a valid image in SRAM and we didn't just copy it from NOR, store it to the stale region
        err = uh_store_update_image();
        // ToDo: when do we swap stale/active state?
    }


    if(!err) {
        err = uh_send_update_to_clients(DEVICE_MVCP);
    }

    if(!err) {
        err = uh_send_update_to_clients(DEVICE_ACP);
    }

    /* Don't need this for the update region
    if(!err) {
        err = uh_update_region_erase();
        TraceE3(TrcMsgErr3, "region erase, err = %d\n", err, 0,0,0,0,0);
    }
    */

    return err;
}

