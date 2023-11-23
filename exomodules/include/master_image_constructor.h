/**
 * @file    master_image_constructor.h
 *
 * @brief   Data structures for the master image constructor.
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

#ifndef MASTER_IMAGE_CONSTRUCTOR_H
#define MASTER_IMAGE_CONSTRUCTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "checksum.h"
#include "magic_numbers.h"
#include "storage/storage_memory_interface.h"
#include "thruster_control.h"



typedef struct {
  uint8_t maj;
  uint8_t min;
  uint8_t rev;
  uint8_t unused;
} sw_version_nums_t;

typedef struct {
  char *path;
  uint32_t component;
  uint32_t magic_number;
  uint32_t git_sha;
  uint8_t maj;
  uint8_t min;
  uint8_t rev;
} ManifestFile_t;

#pragma pack(push)                  /* push current alignment to stack              */
#pragma pack(1)                     /* set alignment to 1 byte boundary             */

typedef struct {                    /* UpdateImageHdrInfo_t = 36 bytes              */
  uint32_t target_magic;            /* 0x00-0x03 Magic number for target device     */
  uint32_t offset;                  /* 0x04-0x07 Offset within the update file      */
  uint32_t size;                    /* 0x08-0x0B Size of the component              */
  uint32_t device_id;               /* 0x0C-0x0F Device ID(device_type_t)           */
  uint8_t  major;                   /* 0x10-0x11 Image major version                */
  uint8_t  minor;                   /* 0x11-0x12 Image major version                */
  uint8_t  rev;                     /* 0x12-0x13 Image major version                */
  uint8_t  unused;                  /* 0x13-0x14 Padding                            */
  uint8_t  git_sha[16];             /* 0x14-0x23 git rev-parse HEAD (first 16)      */
} UpdateImageHdrInfo_t;

typedef struct {                    /* UpdateImageHdr_t = 64 Bytes                  */
  UpdateImageHdrInfo_t image_info;  /* 0x00-0x23 Component info                     */
  uint8_t  unused[0x1A];            /* 0x24-0x39 Pad = size-pos-(CRC + Magic)       */
  uint16_t crc;                     /* 0x3E-0x3F CRC for this component             */
} UpdateImageHdr_t;

typedef struct {                    /* UpdateFileHeaderInfo_t = 8 Bytes             */
  uint32_t magic_number;            /* 0x000-0x003 Magic Number for sanity          */
  uint16_t header_size;             /* 0x004-0x005 Size of header                   */
  uint16_t num_images;              /* 0x006-0x007 Number images included           */
} UpdateFileHeaderInfo_t;

typedef struct {                    /* UpdateFileHeader_t = 256 Bytes               */
  UpdateFileHeaderInfo_t info;      /* 0x000-0x007 Info about header                */
  UpdateImageHdr_t image_hdr[DEVICE_MAXIMUM]; /* Currently 3 = 192 bytes  0x008-0x0C7  */
  uint8_t  reserve2[54];            /* 0x0C8-0x0FD Padding                          */
  uint16_t crc;                     /* 0x0FE-0x0FF CRC for Error Checking           */
} UpdateFileHeader_t;



#pragma pack(pop)

#endif //MASTER_IMAGE_CONSTRUCTOR_H
