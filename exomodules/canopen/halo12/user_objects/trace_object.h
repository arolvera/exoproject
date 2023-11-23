/**
 * @file    trace_object.h
 *
 * @brief   ??? Register the trace object (0x5001) dictionary entries.
 *
 * We register 11 objects but only document 7. Why?
 * Document it
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

#ifndef CO_TRACE_USER_OBJECT_H
#define CO_TRACE_USER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



/**
 * @brief Register the trace object (0x5001) dictionary entries.
 * @param self pointer to dynamic object dictionary 
 */
void TraceDebugOD(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* CO_TRACE_USER_OBJECT_H */
