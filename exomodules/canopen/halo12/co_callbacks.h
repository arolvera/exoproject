/**
 * @file    co_callbacks.h
 *
 * @brief   ??? What is the purpose of this? We just appear to be overriding
 * functions defined in the canopen stack with empty functions. And only one
 * is publicly exposed in the header, the rest are public in the c file.
 *
 * Custom implementations of Can open message callbacks.
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

#ifndef CO_CALLBACKS_H
#define CO_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


    
void COTmrLockCreate(void);



#ifdef __cplusplus
}
#endif

#endif /* CO_CALLBACKS_H */

