/**
 * @file:   user_object_config.h
 *
 * @brief:  ??? Definition for which user objects will be configured for a given
 * system. There should be one entry, an init function, that takes the OD
 * pointer and populates the user objects according to the project definition.
 * Project definition should be compile time switches.
 *
 * What are the key things left to complete?
 *
 * @copyright   Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef USER_OBJECT_CONFIG_H
#define USER_OBJECT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_dict.h"



/**
 * @brief Add neccessary entries into the Dynamic Object Dictionary for all user
 * object for the defined project configuration.
 * @param self pointer to dynamic object dictionary 
 */
void user_object_init(OD_DYN *self);



#ifdef __cplusplus
}
#endif

#endif /* USER_OBJECT_CONFIG_H */

