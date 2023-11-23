// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com
//

#ifndef MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_INCLUDE_HAL_H_
#define MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_INCLUDE_HAL_H_

#include "msg_callback.h"

#define MCAN_EXTERN_HANDLE 0
#define MCAN_CLIENT_HANDLE 1


typedef enum
{
    CAN_DLC_0 = 0,
    CAN_DLC_1 = 1,
    CAN_DLC_2 = 2,
    CAN_DLC_3 = 3,
    CAN_DLC_4 = 4,
    CAN_DLC_5 = 5,
    CAN_DLC_6 = 6,
    CAN_DLC_7 = 7,
    CAN_DLC_8 = 8,
} can_dlc_t;


#endif//MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_INCLUDE_HAL_H_
