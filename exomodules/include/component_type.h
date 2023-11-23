/**
 * @file    component_type.h
 *
 * @brief   Component type enum
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

#ifndef COMPONENT_TYPE_H
#define COMPONENT_TYPE_H

#ifdef	__cplusplus
extern "C" {
#endif



typedef enum {
    COMPONENT_SYSTEM_CONTROL = 0,
    COMPONENT_KEEPER,
    COMPONENT_ANODE,
    COMPONENT_MAGNET,  /* Single magnet components use OUTER IDs */
    COMPONENT_MAGNET_I,
    COMPONENT_VALVES,
    COMPONENT_MAXIMUM,
} component_type_t;

typedef enum {
    DEVICE_ECPK = 0,
    DEVICE_MVCP,
    DEVICE_ACP,
    DEVICE_MAXIMUM,
} device_type_t;



#endif //COMPONENT_TYPE_H
