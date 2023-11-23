/**
 * @file    update_helper.c
 *
 * @brief   This module does the work of updating the system.
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

#include <stdint.h>
#include <string.h>
#include "storage/storage_helper.h"
#include "storage/storage_memory_layout.h"
#include "trace/trace.h"
#include "update_helper.h"
#include "client-update-server/client_update_server.h"


/* Best size for read/writes @todo - get this from the storage device */
#define UH_BLOCK_SIZE 512

#define SM_FILE_PERMS   S_IWUSR|S_IWGRP|S_IRUSR|S_IRGRP|S_IROTH

#define MODULE_NUM MODULE_NUM_UPDATE_COMMAND
#define SUBMODULE_NUM UPDATE_COMMAND_SUBMODULE



#ifndef WATCHDOG_CLEAR
/* For shared modules that do not have a watchdog */
#define WATCHDOG_CLEAR()
#endif

/**
 * Calculate the 16 bit CRC for the given size
 * @param fd open fd (file pos at zero)
 * @param size the size to check
 * @param crc pointer to store crc
 * @return 0 on success or -1 on failure
 */
static int uh_image_crc_calc(int fd, int size, uint16_t *crc)
{
    int err = 0;
    int bytes_read = 0;
    int total_read = 0;
    int remaining = size;
    
    uint8_t buf[UH_BLOCK_SIZE] = {0};
    int block = 0;

    storage_memory_lseek(fd, 0, SEEK_CUR);
    
    do{
        block = remaining > UH_BLOCK_SIZE ? UH_BLOCK_SIZE : remaining;
        bytes_read = storage_memory_read(fd, &buf[0], block);
        storage_memory_lseek(fd, 0, SEEK_CUR);
        if(bytes_read < 0) {
            err = __LINE__;
            TraceE3(TrcMsgErr3, "Read failed with err | line: %d\n", err, 0,0,0,0,0);
        } else {
            for(int i = 0; i < bytes_read; i++){
                *crc = update_crc_16(*crc, buf[i]);
            }
            total_read += bytes_read;
            remaining -= bytes_read;
        }
    } while(bytes_read > 0 && total_read < size && !err);
    if(total_read != size) {
        err = __LINE__;
        TraceE3(TrcMsgErr3, "Not enough bytes read | size:%d | read:%d | line: %d \n",size, total_read,err,0,0,0);
    }
    return err;
}


int uh_image_crc_calc16(int fd, int size, uint16_t *crc)
{
    int err = uh_image_crc_calc(fd, size, (uint16_t*)crc);
    return err;
}

int uh_crc16_check(uint8_t *buf, int size, uint16_t match_crc)
{
    int err = 0;
    volatile uint16_t crc = 0;
    volatile uint16_t match = match_crc;
    for(int i = 0; i < size; i++){
        crc = update_crc_16(crc, buf[i]);
    }
    if(match == crc) { err = 0; }
    else { err = -1; }
    if(err) {
        TraceE3(TrcMsgErr3, "CRC mismatch calc crc:%x match crc:%x",
                crc, match_crc,0,0,0,0);   
    }
    return err;
}

int uh_crc32_check(uint8_t *buf, int size, uint32_t match_crc)
{
    int err = 0;
    volatile uint32_t crc = 0;
    volatile uint32_t match = match_crc;
    for(int i = 0; i < size; i++){
        crc = update_crc_32(crc, buf[i]);
    }
    if(match == crc) { err = 0; }
    else { err = -1; }
    if(err) {
        TraceE3(TrcMsgErr3, "CRC mismatch calc crc:%x match crc:%x", crc, match_crc,0,0,0,0);
    }
    return err;
}

/**
 * @brief Calculate the crc for a binary image region and compare to the value stored
 * in the update image header. A full update image must exist in the SRAM Update region.
 * @param device index matching a device in the device_type_t enum
 * @param image_size Size of the image file
 * @param match_crc CRC stored in the update image header
 * @return 0 = successful match, non-zero otherwise
 */
static int uh_image_crc_inspect(device_type_t device, uint32_t image_size, uint16_t match_crc)
{
    int err = 0;
    int fd;
    uint32_t offset = 0;

    // get the offset to the device image
    sh_get_device_image_offset(device, 0, &offset);

    // Open the image location to compute the crc
    fd = storage_memory_open((const char *)(UPDATE_IMAGE), O_RDONLY);
    err = storage_memory_lseek(fd, (int)offset, SEEK_SET);

    if(fd < 0) {
        err = __LINE__;
        TraceE3(TrcMsgUpdate, "Failed to open sram memory location for device image | line: %d", err,0,0,0,0,0);
        return err;
    }

    uint16_t calc_crc = 0;
    err = uh_image_crc_calc16(fd, (int)image_size, &calc_crc);

    if(!err) {
        if(calc_crc != match_crc) {
            err = __LINE__;
            TraceE3(TrcMsgErr3, "Header CRC check failed for device, device: %d, line: %d, calc: %d, actual: %d", device, err,calc_crc,match_crc,0,0);
        }
    }

    storage_memory_close(fd);

    return err;
}


static int uh_update_header_get(int fd, UpdateFileHeader_t* ufh)
{
    int err = 0;
    int ufh_size = sizeof(UpdateFileHeader_t);
    err = storage_memory_read(fd, ufh, ufh_size);

    if(err == ufh_size) {
        err = 0;
        if (ufh->info.magic_number != MAGIC_NUMBER_UPDATE_HDR) {
            err = __LINE__;
            TraceE3(TrcMsgErr3, "Invalid magic number | line: %d", err, 0, 0, 0, 0, 0);
        }

        if (!err) {
            err = uh_crc16_check((uint8_t*)ufh, ufh_size - (int) sizeof(uint16_t), ufh->crc);
        }
    }
    return err;
}


static int uh_memcpy(int dest_fd, int src_fd, size_t n)
{
    int err = 0;
    if(dest_fd < 0 || src_fd < 0){
        err = -1;
    }

    if(!err){
        for(size_t i = 0; i < n && !err; i++) {
            uint8_t data = 0;
            err = storage_memory_read(src_fd, &data, 1);
            if (err == 1) {
                err = storage_memory_write(dest_fd, &data, 1);
                if (err == 1) {
                    err = 0;
                }
            }
        }
    }

    return err;
}


static int uh_image_move(int dest_fd, int src_fd)
{
    int err = 0;
    UpdateFileHeader_t header = {0};

    err = uh_update_header_get(src_fd, &header);

    if(!err) {
        /* Make sure we start at the beginnings */
        storage_memory_lseek(src_fd, 0, SEEK_SET);
        storage_memory_lseek(dest_fd, 0, SEEK_SET);
        size_t image_size = header.image_hdr[0].image_info.size + sizeof(header);
        err = uh_memcpy(dest_fd, src_fd, image_size);
    }
    if(!err) {
        err = uh_update_crc_inspect();
        if(err) {
            err = __LINE__;
            TraceE3(TrcMsgErr3, "Update failed CRC inspect | line: %d", err,0,0,0,0,0);
        }
    }

    return err;
}


int uh_update_crc_inspect(void)
{
    int err = 0;
    int fd;
    UpdateFileHeader_t header = {0};

    TraceDbg(TrcMsgUpdate,"Start Update Image CRC checks", 0,0,0,0,0,0);

    fd = storage_memory_open((const char *)UPDATE_IMAGE, O_RDONLY);

    err = uh_update_header_get(fd, &header);

    if(!err) {
        for(uint8_t i = 0; i < (uint8_t)DEVICE_MAXIMUM; i++) {
            if(header.image_hdr[i].image_info.target_magic != MAGIC_NUMBER_ECPK) {
                err = __LINE__;
                TraceE3(TrcMsgErr3, "Invalid image magic number | line: %d", err, 0,0,0,0,0);
                break;
            }
            err = uh_image_crc_inspect(i, header.image_hdr[i].image_info.size, header.image_hdr[i].crc);
            if(err) {
                // crc check failed, erase the uploaded update image
//                uh_update_region_erase();
                break;
            }
        }
    }

    if(!err) {
        TraceDbg(TrcMsgUpdate,"Update Image passed CRC inspect", 0,0,0,0,0,0);
    }
    /* No matter what - if it is open, close it */
    if(fd >= 0) {
        storage_memory_close(fd);
    }

    return err;
}


int uh_update_region_erase(void)        // ToDo: Is there a better way in the driver to erase?
{
    int err = 0;
    uint8_t data[1024] = {0};        // we'll write 0's an arbitrary 1024 bytes at a time
    const int DATA_SIZE = sizeof(data);

    int fd = storage_memory_open((const char *)UPDATE_IMAGE, O_WRONLY);

    if(fd < 0) {
        err = __LINE__;
        TraceE3(TrcMsgErr3, "SRAM Update region failed to open | line: %d", err, 0,0,0,0,0);
    } else {
        for(int j = 0; j < UPDATE_REGION_SIZE/DATA_SIZE; j++) {
            err = storage_memory_write(fd, data, DATA_SIZE);
            if(err == DATA_SIZE) {
                err = 0;
            }
            else {
                err = __LINE__;
                TraceE3(TrcMsgErr3, "Failed to write to SRAM update region | line: %d", err, 0,0,0,0,0);
                break;
            }
        }
    }
    storage_memory_close(fd);

    return err;
}


typedef enum{
    UH_UPDATE_ACTION_UPLOAD_STALE,
    UH_UPDATE_ACTION_STORE_UPDATE_IMAGE,
    UH_UPDATE_ACTION_INSTALL_ECPK_IMAGE,
    UH_UPDATE_ACTION_INSTALL_EOL
}UH_UPDATE_ACTION_t;


static int uh_update_action(UH_UPDATE_ACTION_t action)
{
    int src_fd = -1;
    int dest_fd = -1;
    volatile app_stat_t app_stat = {0};
    int err = 0;
    char* rgn = NULL;
    switch(action){
        case UH_UPDATE_ACTION_UPLOAD_STALE:
            sh_app_stat_get(&app_stat);
            if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP1_START_ADDR){
                app_stat.app_status.active_bckp_region = NOR_UPDATE_BKUP2_START_ADDR;
                rgn = UPDATE_IMAGE_BKUP2;
            } else if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP2_START_ADDR){
                app_stat.app_status.active_bckp_region = NOR_UPDATE_BKUP1_START_ADDR;
                rgn = UPDATE_IMAGE_BKUP1;
            }

            if(rgn != NULL) {
                sh_app_stat_set(&app_stat);
                src_fd = storage_memory_open((const char *) rgn, O_RDONLY);

                dest_fd = storage_memory_open((const char *) UPDATE_IMAGE, O_WRONLY);
            }
            break;

        case UH_UPDATE_ACTION_STORE_UPDATE_IMAGE:
            sh_app_stat_get(&app_stat);
            /* store image to backup region */
            if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP1_START_ADDR){
                app_stat.app_status.active_bckp_region = NOR_UPDATE_BKUP2_START_ADDR;
                rgn = UPDATE_IMAGE_BKUP2;
            } else if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP2_START_ADDR){
                app_stat.app_status.active_bckp_region = NOR_UPDATE_BKUP1_START_ADDR;
                rgn = UPDATE_IMAGE_BKUP1;
            }

            if(rgn != NULL) {
                sh_app_stat_set(&app_stat);
                dest_fd = storage_memory_open((const char *) rgn, O_WRONLY);

                src_fd = storage_memory_open((const char *) UPDATE_IMAGE, O_RDONLY);
            }
            break;

        case UH_UPDATE_ACTION_INSTALL_ECPK_IMAGE:
            src_fd = storage_memory_open((const char *)UPDATE_IMAGE, O_RDONLY);

            dest_fd = storage_memory_open((const char *)FRAM_UPDATE_IMAGE, O_WRONLY);
            break;
        default :
            err = __LINE__;
            break;
    }

    if(!err){
        err = uh_image_move(dest_fd, src_fd);
    }

    storage_memory_close(dest_fd);
    storage_memory_close(src_fd);



    return err;
}


int uh_upload_stale_update(void)
{
    int err = 0;

    err = uh_update_action(UH_UPDATE_ACTION_UPLOAD_STALE);

    return err;
}


int uh_store_update_image(void)
{
    int err = 0;

    err = uh_update_action(UH_UPDATE_ACTION_STORE_UPDATE_IMAGE);

    return err;
}


int uh_install_ecpk_image(void)
{
    int err = 0;

    err = uh_update_action(UH_UPDATE_ACTION_INSTALL_ECPK_IMAGE);

    return err;
}


int uh_send_update_to_clients(uint8_t device_type)
{
    int err = 0;
    int client_id = -1;
    switch(device_type) {
        case DEVICE_MVCP:
            client_id = COMPONENT_MAGNET;
            break;
        case DEVICE_ACP:
            client_id = COMPONENT_ANODE;
            break;
        default:
            TraceE3(TrcMsgErr3, "Invalid client device type specified for update: %d", device_type, 0,0,0,0,0);
            err = __LINE__;
    }

    if(!err){
        #ifndef BUILD_MASTER_IMAGE_CONSTRUCTOR
        cus_client_prepare(client_id);
        #endif
        // client reset
    }

    return err;
}
