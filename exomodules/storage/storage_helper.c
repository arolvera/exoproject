/**
 * @file    storage_helper.c
 *
 * @brief   Helper functions for accessing 'things' with multiple redundant
 * copies in memory.
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

#include "storage/storage_helper.h"
#include "trace/trace.h"
#include "utils/macro_tools.h"
#include "storage_memory_layout.h"
#include "master_image_constructor.h"
#include "string.h"
#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

// ToDo: Delete commented code

/**
 * @brief active_fw_storage_region identifies which region contains the firmware image
 * currently installed in FRAM. Values are limited to 1 or 2.
 */
static uint8_t active_fw_storage_region = 1;


static int sh_app_config_status_init(void)
{
    int err = 0;

    volatile app_stat_t app_stat = {0};
    err = sh_app_stat_get(&app_stat);
    /* Don't check err on first init */
    if(app_stat.app_status.magic_number != MAGIC_APP_STAT_SIG){
        memset((app_stat_t*)&app_stat, 0, sizeof(app_stat_t));
        app_stat.app_status.magic_number = MAGIC_APP_STAT_SIG;
        app_stat.app_status.active_bckp_region = NOR_UPDATE_BKUP1_START_ADDR;
        sh_app_stat_set(&app_stat);
    }

    return err;
}


int sh_app_stat_get(volatile app_stat_t* app_stat)
{
    int err = 0;
    int rd_fd = -1;
    if(app_stat == NULL){
        err = __LINE__;
    } else {
        rd_fd = storage_memory_open(APP_CONFIGURATION, O_RDONLY);
        if (rd_fd < 0) {
            err = __LINE__;
        } else {
            int n = storage_memory_read(rd_fd, (app_stat_t*)app_stat, sizeof(app_stat_t));
            if (n != sizeof(app_stat_t)) {
                err = __LINE__;
            } else {
                uint16_t crc_match = 0;

                for(unsigned int i = 0; i < (sizeof(app_stat_t) - sizeof(app_stat->crc)); i++){
                    volatile uint8_t data = ((volatile uint8_t*)app_stat)[i];
                    crc_match = update_crc_16(crc_match, data);
                }

                if(app_stat->crc != crc_match){
                    err = __LINE__;
                }
            }
        }
        storage_memory_close(rd_fd);
    }
    return err;
}


int sh_app_stat_set(volatile app_stat_t* app_stat)
{
    int err = 0;
    int wr_fd = -1;
    wr_fd = storage_memory_open(APP_CONFIGURATION, O_WRONLY);
    if(wr_fd < 0){
        err = __LINE__;
    } else {
        uint16_t crc = 0;

        for(unsigned int i = 0; i < (sizeof(app_stat_t) - sizeof(app_stat->crc)); i++){
            crc = update_crc_16(crc, ((uint8_t*)app_stat)[i]);
        }

        app_stat->crc = crc;
        for(unsigned int i = 0; i < sizeof(app_stat_t) && !err; i++) {
            int n = storage_memory_write(wr_fd, &((uint8_t *)app_stat)[i], sizeof(uint8_t));
            if(n != sizeof(uint8_t)){
                err = __LINE__;
            }
        }

    }

    storage_memory_close(wr_fd);

    return err;
}


int sh_get_device_image_offset(device_type_t device, uint8_t mem_opt, uint32_t* offset)
{
    int err = 0;
    int fd;
    UpdateFileHeader_t header = {0};
    int hdr_size = sizeof(UpdateFileHeader_t);

    if(mem_opt == 0) {
        fd = storage_memory_open((const char *)UPDATE_IMAGE, O_RDONLY);
    } else {
        if(active_fw_storage_region == 1) {
            fd = storage_memory_open((const char *)UPDATE_IMAGE_BKUP1, O_RDONLY);
        } else {
            fd = storage_memory_open((const char *)UPDATE_IMAGE_BKUP2, O_RDONLY);
        }
    }

    if(fd < 0) {
        err = __LINE__;
        TraceE3(TrcMsgUpdate, "Failed to open sram memory location for crc check", 0,0,0,0,0,0);
        return err;
    }

    err = storage_memory_read(fd, &header, hdr_size);
    if(err != hdr_size){
        err = __LINE__;
        TraceE3(TrcMsgUpdate, "Failed to read the update header", 0,0,0,0,0,0);
        return err;
    }
    *offset = header.image_hdr[device].image_info.offset;

    storage_memory_close(fd);

    return err;
}

int sh_get_active_versions(device_type_t device, uint32_t* version, uint32_t* git_sha)
{
    int err = 0;
    int fd = -1;
    UpdateFileHeader_t hdr = {0};
    int hdr_size = sizeof(UpdateFileHeader_t);

    if(active_fw_storage_region == 1) {
        fd = storage_memory_open((const char *)UPDATE_IMAGE_BKUP1, O_RDONLY);
    } else {
        fd = storage_memory_open((const char *)UPDATE_IMAGE_BKUP2, O_RDONLY);
    }

    if(fd >= 0){
        err = storage_memory_read(fd, &hdr, hdr_size);
        storage_memory_close(fd);
    } else{
        err = -1;
    }

    if(err == hdr_size){
        err = 0;
        *version = VERSION(hdr.image_hdr[device].image_info.major,
                           hdr.image_hdr[device].image_info.minor,
                           hdr.image_hdr[device].image_info.rev);
        *git_sha = *((uint32_t*)hdr.image_hdr[device].image_info.git_sha);
    }

    return err;
}

uint8_t sh_get_software_active_backup_region(void)
{
    return active_fw_storage_region;
}

void sh_swap_software_backup_region(void)
{
    if(active_fw_storage_region == 1) {
        active_fw_storage_region = 2;
    } else {
        active_fw_storage_region = 1;
    }
}

int sh_init(void)
{
    int err = sh_app_config_status_init();

    return err;
}