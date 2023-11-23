/**
 * @file    client_service.h
 *
 * @brief   ??? What is the purpose of autostart and stop?
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

#ifndef CONTROL_AUTOSTART_H
#define	CONTROL_AUTOSTART_H

#ifdef	__cplusplus
extern "C" {
#endif



/**
 * @brief
 */
void ctrl_autostop(void);

/**
 * @brief
 * @param setpoint
 * @return
 */
int ctrl_autostart(uint32_t setpoint);



#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_AUTOSTART_H */

