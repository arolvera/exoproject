/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#ifndef CLIENT_CONTROL_USV_H
#define	CLIENT_CONTROL_UCV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>


/*******************************************************************************
Safety limits checks 
*******************************************************************************/
#pragma pack(push,1)
typedef struct _input_voltage_range {
    int low;
    int high;
    int margin;
} input_voltage_range_t;

typedef struct _keeper_septic_limits {
    int critical;
    int warn;
    int clear;
} keeper_septic_limits_t;
    
typedef struct _safety_check_limits {
    keeper_septic_limits_t keeper_limits;
    int ss_power_high_shutdown;
    int ss_power_high_warning;
    int ss_power_high_clear;
    int ss_power_low_warning;
    int ss_power_low_clear;
    int magnet_current_error;
    input_voltage_range_t vin_range;
} safety_check_limits_t;

//user config vars for client control
typedef struct{
    int lockout_time;
    int mci_hci_misses_allowed;
    safety_check_limits_t limits;
    uint8_t serial_baud;
    int lockout_save_time;
}cc_usv_t;
#pragma pack(pop)

cc_usv_t *cc_usv_get(int);
safety_check_limits_t *client_limits_safety_limits_get(void);
/*******************************************************************************
 * Client control params
*******************************************************************************/
void cc_usv_lockout_time_set(int time_us);
void cc_usv_hci_misses_alowed_set(int misses_allowed);

int cc_usv_lockout_time_get(void);
int cc_usv_lockout_save_time_get(void);
int cc_usv_hci_misses_alowed_get(void);

/*******************************************************************************
Safety limits checks 
*******************************************************************************/

void cc_usv_limit_keeper_overvoltage_set(keeper_septic_limits_t limits);
void cc_usv_limit_ss_power_limit_set(int mw);
void cc_usv_limit_ss_power_high_set(int mw);
void cc_usv_limit_ss_power_low_set(int mw);
void cc_usv_limit_magnet_error_set(int err);
void cc_usv_limit_input_power_set(input_voltage_range_t r);

int cc_usv_limit_ss_power_limit_get(void);
int cc_usv_limit_ss_power_high_get(void);
int cc_usv_limit_ss_power_low_get(void);
uint16_t cc_usv_limit_magnet_error_get(void);

input_voltage_range_t  cc_usv_limit_input_power_get(void);
keeper_septic_limits_t cc_usv_limit_keeper_overvoltage_get(void);
    

/*******************************************************************************
 * Serial Baud
*******************************************************************************/
enum{
    CC_UCV_UART_BAUD_115200 = 0x00,
    CC_UCV_UART_BAUD_921600 = 0x11
};

void cc_usv_serial_baud_set(uint8_t baud);
uint8_t cc_usv_serial_baud_get(void);


#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_CONTROL_UCV_H */

