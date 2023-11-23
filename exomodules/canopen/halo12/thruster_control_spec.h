/**
 * @file    co_project_spec.h
 *
 * @brief   ??? Create the object directory for the project.
 *
 * This is duplicated in co_project_spec. Can we delete the other? Yes
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

#ifndef THRUSTER_CONTROL_SPEC_H
#define THRUSTER_CONTROL_SPEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_core.h"
//#include "thruster_control.h"



extern struct CO_NODE_SPEC_T AppSpec;

/**
 * @brief Create the object directory for the project.
 * @param self pointer to dynamic object dictionary 
 */
CO_OBJ *CreateDir();



#ifdef __cplusplus
}
#endif

#endif /* THRUSTER_CONTROL_SPEC_H */
