/**
 * @file    client_health.h
 *
 * @brief   ??? Functions to check client power and voltage levels.
 *
 * Why aren't these in health.h or vice versa?
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

#ifndef CLIENT_HEALTH_H
#define	CLIENT_HEALTH_H

#ifdef	__cplusplus
extern "C" {
#endif



/**
 * @brief
 * @return
 */
int client_vin_check(void);

/**
 * @brief
 * @return
 */
int client_setpoint_power_check(void);



#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_HEALTH_H */

