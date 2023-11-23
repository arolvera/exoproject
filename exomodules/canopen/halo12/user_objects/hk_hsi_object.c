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
/**
 * hk_hsi_object.c
 *
 * @Company Exoterra
 * @File hk_hsi_object.c
 * @Summary  User object for HK ADC reading.
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */

#include "hk_hsi_object.h"
#include "hsi_memory.h"
#include "user_object_od_indexes.h"


typedef enum {
    HK_HSI_DOMAIN_SUBIDX_COUNT,
    HK_HSI_DOMAIN_SUBIDX_DOMAIN_OBJ,
} OD_HKM_HSI_DOMAIN;

CO_OBJ_DOM hsi_domain = {
    .Offset = 0,
    .Size = sizeof(client_telemetry_t),
    .Start = (uint8_t*)&hsi_mem,
};
#define CO_HSI_MEM_READ  ((uintptr_t)&hsi_domain)

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
void HkHsiOD(OD_DYN *self)
{    
    ODAdd(self, CO_KEY(OD_INDEX_HSI_MEM, HK_HSI_DOMAIN_SUBIDX_COUNT, CO_UNSIGNED16|CO_OBJ_D__R_), 0, 1);
    ODAdd(self, CO_KEY(OD_INDEX_HSI_MEM, HK_HSI_DOMAIN_SUBIDX_DOMAIN_OBJ, CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_HSI_MEM_READ);
}
/* *****************************************************************************
 End of File
 */
