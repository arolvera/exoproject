/**
 * @file    co_project_spec.h
 *
 * @brief   ??? Create the object directory for the project.
 *
 * Why do we have this?
 *   CreateDir and AppSpec are also defined in thruster_control_spec
 *   The c file is not referenced in cmake
 *
 * We can delete thsi
 *
 * Why are we using __THRUSTER_CONTROL_SPEC_H_ in this file? Copy and paste error?
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

#ifndef __THRUSTER_CONTROL_SPEC_H_
#define __THRUSTER_CONTROL_SPEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "co_core.h"



extern struct CO_NODE_SPEC_T AppSpec;

/**
 * @brief Create the object directory for the project.
 * @param self pointer to dynamic object dictionary 
 */
CO_OBJ *CreateDir();



#ifdef __cplusplus
}
#endif

#endif /* __THRUSTER_CONTROL_SPEC_H_ */
