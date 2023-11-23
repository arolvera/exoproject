/**
 * @file    hsi_memory.h
 *
 * @brief   Expose a global variable for collecting telemetry data.
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

#ifndef HSI_MEMORY_H
#define	HSI_MEMORY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ext_decl_define.h"
#include "diag.h"



EXT_DECL client_telemetry_t hsi_mem;



#ifdef	__cplusplus
}
#endif

#endif	/* HSI_MEMORY_H */

