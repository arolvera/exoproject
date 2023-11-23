/**
 * @file    client_error_details.h
 *
 * @brief   ??? Data structures for tracking client error details.
 *
 * What do the push and pack pragmas do? Removes padding
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

#ifndef CLIENT_ERROR_DETAILS_H
#define CLIENT_ERROR_DETAILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "error/error_handler.h"

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */



typedef struct {
    uint16_t keeper_voltage;
    uint16_t lf_pressure;
} keeper_ov_info_t;

typedef struct {
    uint32_t anode_status;
    uint32_t anode_adc_val;
} anode_specific_error_detail_t;

typedef struct {
    base_error_detail_t b_d;
    anode_specific_error_detail_t anode_specific_error;
}  anode_control_error_detail_t;

typedef struct {
    uint32_t keeper_status;
    uint32_t keeper_adc_val;
} keeper_specific_error_detail_t;

typedef struct {
    uint32_t magnet_status;
    uint32_t magnet_adc_val;
} magnet_specific_error_detail_t;

typedef struct {
    uint32_t power_level;
    uint32_t setpoint;
} power_specific_error_detail_t;

typedef union {
    uint32_t boot_status;
    uint32_t voltage;
    uint32_t err;
    keeper_ov_info_t keeper_ov;
    power_specific_error_detail_t  power_level;
    anode_specific_error_detail_t  anode_specific_error_detail;
    keeper_specific_error_detail_t keeper_specific_error_detail;
    magnet_specific_error_detail_t magnet_specific_error_detail;
} client_control_specific_detail_t;



#pragma pack(pop)                  /* restore original alignment from stack   */

#ifdef __cplusplus
}
#endif

#endif  //CLIENT_ERROR_DETAILS_H
