/**
* @file    firmware_versions_object.c
*
* @brief   Implementation for the Firmware Versions Object Dictionary entry.
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

#include "firmware_versions_object.h"
#include "utils/macro_tools.h"
#include "master_image_constructor.h"
#include "user_object_od_indexes.h"
#include "keeper/control_keeper.h"
#include "valve/control_valves.h"
#include "control_anode.h"
#include "magnet/control_magnets.h"
#include "thruster_control_version.h"
#include "task-monitor/component_tasks.h"
#include "client-control/client_control.h"



#define VERSION_GIT_SHA_COLS            2

#define VERSION_INDEX                   0
#define GIT_SHA_INDEX                   1



/* 
 * 2-D array of pointers to point to device version info and Git SHA.
 */
static uint32_t* firmware_versions[COMPONENT_MAXIMUM][VERSION_GIT_SHA_COLS];

static CO_ERR FirmwareVersionsRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR FirmwareVersionsWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

/* Index (device_id) for array assignment*/
static uint8_t device_index;

/* Boot loader version */
//static uint32_t  bl_version_git_sha[DEVICE_VERSION_GIT_SHA_COLS];

/* System control version */
static uint32_t  sc_version_git_sha[VERSION_GIT_SHA_COLS];

/** 
 * FirmwareVersions Control User Object
 */
CO_OBJ_TYPE FirmwareVersionsType = {
    0,
    0,           
    FirmwareVersionsRead,
    FirmwareVersionsWrite,   /* type function to write object content */
};
#define CO_TFIRMWARE_VERSIONS  ((CO_OBJ_TYPE*)&FirmwareVersionsType)



#ifdef EXORUN_WITH_FIXED_RESID
/*
 * @brief Assign pointers that store version numbers and Git SHA from pre-processor.
 * This function is only needed when running on a dev board without a NOR flash.
 */
static void device_version_ptrs_get(void)
{
    /* Point to variables that will contain device version info */
    ctrl_keeper_version_get(&firmware_versions[COMPONENT_KEEPER][VERSION_INDEX]);
    ctrl_magnets_version_get(&firmware_versions[COMPONENT_MAGNET][VERSION_INDEX]);
    ctrl_valves_version_get(&firmware_versions[COMPONENT_VALVES][VERSION_INDEX]);
    ctrl_anode_version_get(&firmware_versions[COMPONENT_ANODE][VERSION_INDEX]);

    //bl_version_git_sha[GIT_SHA_INDEX] = iacm_get(IACM_TC_BL_GIT_SHA);
    sc_version_git_sha[VERSION_INDEX] = (uint32_t)VERSION(TC_MAJOR_VERSION, TC_MINOR_VERSION, TC_REVISION);
    sc_version_git_sha[GIT_SHA_INDEX] = TC_GIT_SHA;
    firmware_versions[COMPONENT_SYSTEM_CONTROL][VERSION_INDEX] = &sc_version_git_sha[VERSION_INDEX];
    firmware_versions[COMPONENT_SYSTEM_CONTROL][GIT_SHA_INDEX] = &sc_version_git_sha[GIT_SHA_INDEX];
}
#endif
    


/* table_row is a container holding the version and sha for the currently selected device_index.
 * It is referenced by the object dictionary when a firmware versions command is sent. The stored
 * version and sha are updated when the user sends the firmware versions command with a
 * sub-index of 1 (Device Select) */
static uint32_t table_row[2];



/*
 * Write new device index and update pointers for appropriate device data
 */
static CO_ERR FirmwareVersionsWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    /* Update device index */
    device_index = *((uint8_t*)buf);
    
    table_row[VERSION_INDEX] = *firmware_versions[device_index][VERSION_INDEX];
    table_row[GIT_SHA_INDEX] = *firmware_versions[device_index][GIT_SHA_INDEX];
    
    return CO_ERR_NONE;
}


/*
 * Write new device index and update pointers for appropriate device data
 */
static CO_ERR FirmwareVersionsRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    /* Update device index */
    *((uint8_t*)buf) = device_index;
    return CO_ERR_NONE;
}



void FirmwareVersionsOD(OD_DYN *self)
{
    /* Number of OD indices */
    ODAdd(self, CO_KEY(OD_INDEX_FIRMWARE_VERSIONS, 0, CO_UNSIGNED8  |CO_OBJ_D__R_),
          0, 4);

    /*
     * Special handling to avoid having a million subindices for each device.
     *  Writing to this index will adjust the device pointers to the corresponding
     *  row in the device version info column
     */
    ODAdd(self, CO_KEY(OD_INDEX_FIRMWARE_VERSIONS, 1, CO_UNSIGNED8 |CO_OBJ____RW),
          CO_TFIRMWARE_VERSIONS, (uintptr_t)&FirmwareVersionsType);

#ifdef EXORUN_WITH_FIXED_RESID
    /* We are likely running on a dev board without a NOR flash, get the versions from defines */
    device_version_ptrs_get();
#else
    /* We are running on a production board that should have a NOR flash (assuming we are ECPK) */
    if(system_res_id == ECPK_RESID) {
        client_internal_version_OD_ptrs_populate(firmware_versions);
    } else {
//        device_version_ptrs_get();
    }
#endif

    /* Version sub-index */
    table_row[VERSION_INDEX] = *firmware_versions[device_index][VERSION_INDEX];
    ODAdd(self, CO_KEY(OD_INDEX_FIRMWARE_VERSIONS, 2, CO_UNSIGNED32 | CO_OBJ____R_),
          0, (uintptr_t)&table_row[VERSION_INDEX]);

    /* Sha sub-index */
    table_row[GIT_SHA_INDEX] = *firmware_versions[device_index][GIT_SHA_INDEX];
    ODAdd(self, CO_KEY(OD_INDEX_FIRMWARE_VERSIONS, 3, CO_UNSIGNED32 | CO_OBJ____R_),
          0, (uintptr_t)&table_row[GIT_SHA_INDEX]);
}
