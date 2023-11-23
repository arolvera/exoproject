/**
 * @file    main.c
 *
 * @brief   The Master Image Constructor is a command line utility that bundles
 * software updates from binary image data and headers. This file includes the
 * main function and all of the support functions to create, inspect, and
 * verify the update file.
 *
 * In the interest of upload time, only one image is deployed for each device
 * and resistor banks are used to determine behavior. However, the update file
 * is structured to allow different images to be deployed for each device in
 * the future so a header is created for each device and each header points
 * toi the same image.
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

#include <stdio.h>
#include <stdlib.h>     // Defines EXIT_FAILURE and EXIT_SUCCESS
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "master_image_constructor.h"
#include "storage_memory_layout.h"
#include "update/update_helper.h"
#include "trace/trace_flags.h"
#include "trace/trace.h"



/* string holds the file name and path for the binary assigned to each component.
 * The file should be co-located with this executable but 100 Characters are allowed
 * to hold path info. */
char binary_file_name[DEVICE_MAXIMUM][100] = {0};

/* manifest_file_info structure contains the information needed to build a
 * header for each of the components */
ManifestFile_t manifest_file_info[DEVICE_MAXIMUM];

/* number of images processed in the manifest, should be 1 if all devices get the
 * same image or 3 if each device gets a different image. */
uint8_t num_images;

/* Page size to use on the host program  - This may/may not be different on the target */
#define UPDATE_FILE_PAGESIZE 512



/**
 * @brief Read the update file binary and inspect the contents of the header.
 * @param filename File to open
 * @return 0 = success, non-zero otherwise
 */
int inspect_update_file(char* filename)
{
    int err = 0;
    int fd;
    int hdr_size;
    UpdateFileHeader_t header = {0};

    hdr_size = sizeof(header);

    fd = open(filename, O_RDONLY);
    if(fd < 0){
        err = -1;
        printf("%s:%d failed to open file %s:%s\n", __FUNCTION__, __LINE__, filename, strerror(errno));
    }
    if(!err) {
        err = (int)read(fd, &header, hdr_size);
        if(err != hdr_size) {
            printf("%s:%d header read failed:%d:%s\n", __FUNCTION__, __LINE__, err, strerror(errno));
        }
    }

    printf("Header magic number:          0x%x\n", header.info.magic_number);
    printf("Header size:                  0x%x\n", header.info.header_size);
    printf("Header number of images:      0x%x\n", header.info.num_images);
    printf("Header crc:                   0x%x\n", header.crc);

    err = uh_crc16_check((uint8_t *)&header, (int)(hdr_size - sizeof(uint16_t)), header.crc);
    if(!err) {
        printf("Header crc check:             GOOD\n");
    } else {
        printf("Header crc check:             FAIL\n");
        return err;
    }
    printf("\n");

    lseek(fd, offsetof(UpdateFileHeader_t, image_hdr), SEEK_SET);
    for(int i = 0; i < header.info.num_images; i++) {
        UpdateImageHdr_t *comp = &header.image_hdr[i];
        UpdateImageHdrInfo_t *c = &comp->image_info;

        uint16_t match_crc = 0;

        char magic_buf[5] = {0};
        strncpy(magic_buf, (char *) &c->target_magic, 4);

        printf("Component:                %d\n", i);
        printf("Component magic:          0x%08x (%s)\n", c->target_magic, magic_buf);
        printf("Component offset:         0x%x\n", c->offset);
        printf("Component size:           0x%x\n", c->size);
        printf("Component number:         0x%x\n", c->device_id);
        // Major, minor, rev and unused are endian swapped
        printf("Component version:        %d.%d.%d.%d\n",
               c->unused, c->major, c->minor, c->rev);
        printf("Component sha:            %x\n", *(uint32_t*)c->git_sha);
        printf("Component crc:            0x%04x\n", comp->crc);

        lseek(fd, c->offset, SEEK_SET);
        err = uh_image_crc_calc16(fd, (int)c->size, &match_crc);
        if(err) {
            return err;
        } else {
            if(match_crc != comp->crc) {
                printf("Image CRC does not match\n");
                return -1;
            } else {
                printf("Image calc crc:           0x%04x\n", comp->crc);
            }
        }
        printf("\n");
    }
    printf("Update file verifies\n");
    if(fd >= 0) {
        close(fd);
    }
    return err;
}

/**
 * @brief Write the header to the file after computing its crc.
 * @param fd Reference to the update file object
 * @param header Reference to the header object
 * @return Number of bytes written
 */
int update_image_header_write(int fd, UpdateFileHeader_t* header)
{
    int err;
    int crc_size = sizeof(UpdateFileHeader_t) - sizeof(uint16_t);
    unsigned char *buf = (unsigned char*)header;

    for(int i = 0; i < crc_size ; i++) {
        header->crc = update_crc_16(header->crc, buf[i]);
    }

    lseek(fd, 0, SEEK_SET);
    err = (int)write(fd, header, sizeof(UpdateFileHeader_t));
    if(err != sizeof(UpdateFileHeader_t)) {
        printf("%s:%d Write header failed:%d\n", __FUNCTION__, __LINE__, err);
    }

    return err;
}

/**
 * @brief Verify image chunk was written successfully.
 * @param fd Reference to the update file object
 * @param imgBuffer Buffer holder the chunk that was written
 * @param size Size of the chunk
 * @return 0 = success, non-zero otherwise
 */
int update_image_page_verify(int fd, uint8_t* imgBuffer, int size)
{
    int err;

    uint8_t tempPage[size];
    long int currentPosition = lseek(fd, 0, SEEK_CUR);
    lseek(fd, currentPosition - size, SEEK_SET);
    err = (int)read(fd, tempPage, size);
    if(err < 0){
        printf("Page verify read failed with errno 0x%x\n", errno);
        err = errno;
    }
    else if(err != size){
        printf("Incorrect number of bytes read in page verify.  Bytes read = %d, bytes expected = %d\n",
               err, size);
    }
    else if(err == size){
        err = memcmp(tempPage, imgBuffer, size);
        if(err != 0){
            printf("Page match verify failed\n");
        }
    }

    return err;
}

/**
 * @brief Write the referenced image to the update file and compute the offsets,
 * crc, and other info as we go.
 * @param fd Reference to the update file object
 * @param pFile Pointer to the manifest object being processed
 * @param pImgHdr Pointer to the Image Header for the current manifest object
 * @return 0 = success, non-zero otherwise
 */
int process_image(int fd, ManifestFile_t *pFile, UpdateImageHdr_t *pImgHdr)
{
    int err = 0;

    /* Open the image file */
    int imgFd = open(pFile->path, O_RDONLY);
    if(imgFd < 0) {
        err = -1;
        printf("%s:%d open failed: %s:%s\n", __FUNCTION__, __LINE__, pFile->path, strerror(errno));
    }

    /* Get the image file stats */
    if(!err) {
        /* use fstat to get image file size */
        struct stat fileStatus = {0};
        fstat(imgFd, &fileStatus);
        pImgHdr->image_info.size = fileStatus.st_size;
        /* use lseek to get current file position from fd and fill in offset */
        pImgHdr->image_info.offset = lseek(fd, 0, SEEK_CUR);
    }

    /* read in the image file, calulate the CRC, and write to the update file (fd) */
    int bytesRead = 1; // set to 1 just to prime the pump
    int bytesWritten;
    unsigned char imgBuffer[UPDATE_FILE_PAGESIZE] = {0};
    while(!err && bytesRead > 0) {
        bytesRead = (int)read(imgFd, imgBuffer, UPDATE_FILE_PAGESIZE);

        if(bytesRead < 0) {
            err = -1;
            printf("%s:%d Read failed: %s\n", __FUNCTION__, __LINE__, strerror(errno));
        } else {
            for(int i = 0; i < bytesRead; i++){
                pImgHdr->crc = update_crc_16(pImgHdr->crc, imgBuffer[i]);
            }
            bytesWritten = (int)write(fd, imgBuffer, bytesRead);
            if(bytesWritten != bytesRead) {
                err = -1;
                printf("%s:%d Write fail. Bytes read:%d bytes written%d err:%s\n",
                       __FUNCTION__, __LINE__, bytesRead, bytesWritten, strerror(errno));
            } else {
                err = update_image_page_verify(fd, imgBuffer, bytesWritten);
            }
        }
    }

    if(!err) {
        /* Get the current position where the current image ends */
        uint32_t end = lseek(fd, 0, SEEK_CUR);
        uint32_t next = BIN_IMAGE_MAX_SIZE;

        while(end > next) {
            next += BIN_IMAGE_MAX_SIZE;
        }

        lseek(fd, next, SEEK_SET);
        printf("%s:%d end: %x, next: %x\n", __FUNCTION__, __LINE__, end, next);
    }

    /* Don't forget to close the component file and clean up, you silly billy */
    if(imgFd >= 0) {
        close(imgFd);
    }

    return err;
}

/**
 * @brief Print a menu to the console that details argument options.
 * @param prog The name of this executable file (first argument of command)
 */
void help(char *prog)
{
    printf("\n");
    printf("To create an update file:\n");
    printf("\t%s -b <update filename> <manifest file name>\n", prog);
    printf("To inspect an update file:\n");
    printf("\t%s -i <update file name>\n", prog);
    printf("Validate the update file:\n");
    printf("\t%s -v <update file name>\n", prog);
    printf("Create ramp file:\n");
    printf("\t%s -r <size>\n", prog);
    printf("\n");
}

/**
 * @brief Parse the data from each line of the manifest into a data structure
 * that will be used to build headers.
 * @param manifest_filename
 * @return 0 = success, non-zero otherwise
 */
static int parse_manifest(const char* manifest_filename)
{
    int err = 0;
    FILE* fp;
    fp = fopen(manifest_filename, "r");
    if(fp == NULL){
        err = __LINE__;
    }

    int i = 0;
    char data[256] = {0};
    /* Use temp variables to make sscanf happy with %d format specifier */
    int maj_temp = 0;
    int min_temp = 0;
    int rev_temp = 0;

    if(!err){
        /* read each line of the manifest file */
        while(fgets(data, sizeof(data), fp)){
            if(data[0] != '#'){
                sscanf(data, "%[^:]:%d:%8x:%8x:%d:%d:%d",
                        &binary_file_name[i][0],
                        &manifest_file_info[i].component,
                        &manifest_file_info[i].magic_number,
                        &manifest_file_info[i].git_sha,
                        &maj_temp,
                        &min_temp,
                        &rev_temp);

                manifest_file_info[i].path = &binary_file_name[i][0];
                manifest_file_info[i].maj = (uint8_t)maj_temp;
                manifest_file_info[i].min = (uint8_t)min_temp;
                manifest_file_info[i].rev = (uint8_t)rev_temp;
                i++;
                memset(data, 0, sizeof(data));
            }
        }

        num_images = i;

        /* The manifest should have either one image for all three processors,
         * or three images, one for each */
        if(num_images == 1) {
            manifest_file_info[2] = manifest_file_info[1] = manifest_file_info[0];
            manifest_file_info[1].component = 1;
            manifest_file_info[2].component = 2;
        }
        if(num_images != 1 && num_images != 3) {
            err = __LINE__;
        }
    }

    if(fp != NULL){
        fclose(fp);
    }

    return err;
}

/**
 * @brief Build the update binary file. See the docs folder for a graphical
 * layout of the binary file.
 * @param filename Name of the update file to be written
 * @return 0 = success, non-zero otherwise
 */
int build_update(char* filename)
{
    int err = 0;
    int fd;
    uint32_t crc;

    UpdateFileHeader_t header = {0};
    int hdr_size;
    hdr_size = sizeof(header);

    // Populate the main overall header info
    header.info.magic_number = MAGIC_NUMBER_UPDATE_HDR;
    header.info.header_size = hdr_size;
    header.info.num_images = DEVICE_MAXIMUM;     // Always max device number, even if one image is deployed to all three

    // create file with read/write privileges, fail if file already exists
    fd = open(filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR|S_IRGRP|S_IROTH);
    if(fd < 0){
        err = -1;
        printf("%s:%d failed to open file %s:%s\n", __FUNCTION__, __LINE__,
               filename, strerror(errno));
    }
    else{
        /* Seek to the end of the Header region. We will write images to file first, computing offsets
         * and crc as we go. The header will be written last. */
        lseek(fd, HEADER_REGION_SIZE, SEEK_SET);

        // Populate the individual image header info
        for(int i = 0; i < DEVICE_MAXIMUM && !err; i++) {
            ManifestFile_t *pFile = &manifest_file_info[i];
            header.image_hdr[i].image_info.target_magic = pFile->magic_number;
            header.image_hdr[i].image_info.device_id = pFile->component;
            header.image_hdr[i].image_info.major = pFile->maj;
            header.image_hdr[i].image_info.minor = pFile->min;
            header.image_hdr[i].image_info.rev = pFile->rev;
            header.image_hdr[i].crc = 0;
            *((uint32_t*)header.image_hdr[i].image_info.git_sha) = pFile->git_sha;
            printf("%s:%d Build update for device id: %d\n", __FUNCTION__,__LINE__, pFile->component);
            UpdateImageHdr_t *pImgHdr = &header.image_hdr[i];
            if(i < num_images) {
                err = process_image(fd, pFile, pImgHdr);
            } else {
                // we are deploying one image to all devices, so set each header to the same info
                pImgHdr->image_info.size = header.image_hdr[0].image_info.size;
                pImgHdr->image_info.offset = header.image_hdr[0].image_info.offset;
                pImgHdr->crc = header.image_hdr[0].crc;
            }
        }
    }

    // Write the header to the update file
    if(!err) {
        lseek(fd, 0, SEEK_SET);
        err =  update_image_header_write(fd, &header);

        if(err != hdr_size){
            printf("%s:%d Full update image header not written.  Bytes written = %d\n",
                   __FUNCTION__, __LINE__, err);
        }
        else{
            err = update_image_page_verify(fd, (uint8_t*)&header, hdr_size);
            if(err){
                printf("%s:%d Page verify on update image failed.\n", __FUNCTION__,__LINE__);
            }
        }
    }

    if(fd >= 0) {
        close(fd);
    }

    printf("%s:%d err:%d\n", __FUNCTION__, __LINE__, err);

    return err;
}



int main(int argc, char *argv[]) 
{
    if(argc < 3) {
        printf("\nInvalid number of arguments");
        help(argv[0]);
        return EXIT_FAILURE;
    }

    char* mic_file = argv[0];
    char* operation = argv[1];
    char* update_file = argv[2];
    char* manifest_file = argv[3];

    int err = 0;

    if(!strncmp(operation, "-b", 2)) {
        if(argc != 4) {
            printf("\nInvalid number of arguments");
            help(mic_file);
            return EXIT_FAILURE;
        }
        err = parse_manifest(manifest_file);
        if(err) {
            printf("\nError parsing manifest file, check file name and format\n\n");
            return EXIT_FAILURE;
        }
        err = build_update(update_file);
    } else if(!strncmp(operation, "-i", 2)) {
        err = inspect_update_file(update_file);
    } else {
        help(mic_file);
        return EXIT_FAILURE;
    }

#if _TRACE_ENABLE
    /* empty the trace buffer */
    while(traceShowN(10));
#endif

    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

