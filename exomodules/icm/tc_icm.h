/**
 * @file    icm.h
 *
 * @brief   ICM stands for integrity check monitor in the SAMV71 chip.
 *
 * Vorago does not have a similar feature. Can we just delete this?
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

#ifndef TC_ICM_H
#define	TC_ICM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "definitions.h"
#include "error/error_handler.h"
    
typedef enum{
    TC_ICM_SUBMODULE
} TC_ICM_SUBMODULES;

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */
typedef struct tc_icm_specific_error_detail{
    uint32_t eefc_fsr;
    uint32_t digest;
}tc_icm_specific_error_detail_t;
#pragma pack(pop)                  /* restore original alignment from stack   */

void tc_icm_lock_init(void);
void tc_icm_serivce(void);
void tc_icm_disable(void);
void tc_icm_error_handler_init(void);
void tc_icm_verify_repair_enable(void);
void tc_icm_unlock(void);
int tc_icm_lock(void);
size_t tc_icm_error_detail_size_get(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TC_ICM_H */

