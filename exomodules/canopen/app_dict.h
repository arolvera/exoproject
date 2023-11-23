/**
 * @file    app_dict.h
 *
 * @brief   ??? Dictionary management functions to init the dynamic dictionary, add
 * objects, get pointer to the start of the dictionary, and get the size. Can you
 * only add objects? What about changing or removing?
 *
 * Implementation looks complete. Was there anything else to do? No
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

#ifndef APP_DICT_H
#define APP_DICT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_core.h"



typedef struct OD_DYN_T {           /*!< this type represents a dynamic OD   */
    CO_OBJ   *Root;                 /*!< first element of object entry array */
    uint32_t  Len;                  /*!< length of object entry array        */
    uint32_t  Used;                 /*!< number of used object entries       */
} OD_DYN;



/**
 * @brief INIT DYNAMIC OBJECT DICTIONARY
 * @details Set the possible object entries of the dynamic object dictionary
 *          to the given array of object entries (root) with length (len)
 * @param self reference to the dynamic object dictionary
 * @param root pointer to the first element of an object entry array
 * @param len length of the given object entry array
 */
void ODInit(OD_DYN *self, CO_OBJ *root, uint32_t len);

/**
 * @brief ADD DYNAMIC OBJECT ENTRY
 * @details Set or change the type and data of an object entry within the
 *          dynamic object dictionary, addressed by the given object entry
 *          key.
 * @param self reference to the dynamic object dictionary
 * @param key object entry key
 * @param type type of the new or changed object entry
 * @param data data of the new or changed object entry
 * @note You should generate the key with the macro CO_KEY()
 */
void ODAdd(OD_DYN *self, uint32_t key, const CO_OBJ_TYPE *type, uintptr_t data);

/**
 * @brief GET DYNAMIC OBJECT DICTIONARY
 * @details Use this function, when the start address of the object dictionary
 *          is needed.
 * @return Pointer to the first element of the dynamic object dictionary
 */
CO_OBJ *ODGetDict(OD_DYN *self);

/**
 * @brief GET SIZE OF DYNAMIC OBJECT DICTIONARY
 * @details Use this function, when the maximum possible size (including the
 *          mandatory end-marker) of the object dictionary is needed.
 * @return Size of the object array for this dynamic object dictionary
 */
uint32_t ODGetSize(OD_DYN *self);

#ifdef __cplusplus
}
#endif

#endif  // APP_DICT_H
