/**
 * @file    magic_numbers.h
 *
 * @brief   What are these for?
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

#ifndef MAGIC_NUMBERS_H
#define MAGIC_NUMBERS_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------- corresponding ascii encodings
#define MAGIC_NUMBER_UPDATE_HDR         0x72646875 // uhdr
#define MAGIC_NUMBER_ACP                0x78706361 // acpx
#define MAGIC_NUMBER_MVCP               0x7863766d // mvcx
#define MAGIC_NUMBER_ECPK               0x786b6365 // eckx

#define MAGIC_APP_STAT_SIG              0x61737300 // ass\0


/* Magic numbers for Halo 6 images - ToDo: get rid of this */
#define MAGIC_EXECUTION_SYSTEM_CONTROL  0x78656373 //scex
#define MAGIC_EXECUTION_FLIGHT_CONTROL  0x78656366 //fcex
#define MAGIC_EXECUTION_BUCK            0x7865626b //bkex
#define MAGIC_EXECUTION_ANODE           0x78656E61 //anex
#define MAGIC_EXECUTION_KEEPER          0x7865656b //keex
#define MAGIC_EXECUTION_APP_RECOVERY    0x78657261 //arex
#define MAGIC_EXECUTION_MAGNET_O        0x78656f6d //moex
#define MAGIC_EXECUTION_MAGNET_I        0x78656f6d //moex
#define MAGIC_EXECUTION_VALVES          0x78656176 //vaex
#define MAGIC_EXECUTION_BOOTLOADER      0x7865626c //blex
#define MAGIC_EXECUTION_UCV             0x78656375 //ucex

/* Magic numbers for Halo 6 images - ToDo: get rid of this */
#define MAGIC_DATA_SYSTEM_CONTROL       0x61646373 //scda
#define MAGIC_DATA_FLIGHT_CONTROL       0x61646366 //fcda
#define MAGIC_DATA_BUCK                 0x6164626b //bkda
#define MAGIC_DATA_ANODE                0x61646e61 //anda
#define MAGIC_DATA_KEEPER               0x6164656b //keda
#define MAGIC_DATA_APP_RECOVERY         0x61647261 //arda
#define MAGIC_DATA_MAGNET_O             0x61646f6d //moda
#define MAGIC_DATA_MAGNET_I             0x61646f6d //moda
#define MAGIC_DATA_VALVES               0x61646176 //vada
#define MAGIC_DATA_BOOTLOADER           0x61646c62 //blda
#define MAGIC_DATA_UPDATER              0x61647075 //upda
#define MAGIC_DATA_UCV                  0x61646375 //ucda

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif


#endif // MAGIC_NUMBERS_H
