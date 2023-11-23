/**
 * @file    client_control_ucv_object.h
 *
 * @brief   ??? Register the client control object (0x5100) dictionary entries.
 *
 * Why do we have this?
 *   it is commented out of user_object_config
 *   it is not included in cmake
 *   the implementation is broken
 *   it is not documented in either the internal or external ICD
 *
 *  ucv = user calibration variables
* No longer used, need to determine if we will ever need it. Not likely.
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

#ifndef CLIENT_CONTROL_UCV_H
#define	CLIENT_CONTROL_UCV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "app_dict.h"// For adding Oject Dictionary Entires
#include "co_core.h"             // CAN Open core includes



void ClientControlUCVOD(OD_DYN *self);



#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_CONTROL_UCV_H */

