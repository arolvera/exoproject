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

//#include "client_control/client_control_ucv.h"
#include "client_control_usv.h"
#include "keeper/control_keeper.h"
#include "sys/sys_timers.h"

#define CLIENT_DEF_LOCKOUT_TIME (15 MINUTES)
#define CLIENT_DEF_LOCKOUT_SAVE (CLIENT_DEF_LOCKOUT_TIME/3)

/* How many consecutive times an MCU can miss an HSI update before taking action */
#define CONSECUTIVE_MCI_HSI_MISSES_ALLOWED  4

/* Power limits in Milliwatts */
#define INPUT_POWER_HIGH_SHUTDOWN   50000 /* Setpoint expected power plus this  */
#define INPUT_POWER_HIGH_WARNING    30000 /* Setpoint expected power plus this  */
#define INPUT_POWER_HIGH_CLEAR      10000 /* Setpoint expected power plus this  */
#define INPUT_POWER_LOW_WARNING     30000 /* Setpoint expected power minus this */
#define INPUT_POWER_LOW_CLEAR       10000 /* Setpoint expected power minus this */

#define MAGNET_CURRENT_ERROR           35 /* Magnet current error +/-           */

#define INPUT_VOLTAGE_RANGE_LOW        20.0 /* Lower Limint for Voltage         */
#define INPUT_VOLTAGE_RANGE_HIGH       37.0 /* High Limit for Input Voltage     */
#define INPUT_VOLTAGE_MARGIN           01.0 /* Margin for input voltage ranges  */

//user config vars for client control
static cc_usv_t cc_usv = {
    .lockout_time           = CLIENT_DEF_LOCKOUT_TIME,
    .lockout_save_time      = CLIENT_DEF_LOCKOUT_SAVE,
    .mci_hci_misses_allowed = CONSECUTIVE_MCI_HSI_MISSES_ALLOWED,
    .limits.keeper_limits   = { KEEPER_OVER_VOLTAGE_LIMIT,
                                KEEPER_OVER_VOLTAGE_WARN,
                                KEEPER_OVER_VOLTAGE_CLEAR},
    .limits.ss_power_high_shutdown  = INPUT_POWER_HIGH_SHUTDOWN,
    .limits.ss_power_high_warning   = INPUT_POWER_HIGH_WARNING,
    .limits.ss_power_high_clear     = INPUT_POWER_HIGH_CLEAR,
    .limits.ss_power_low_warning    = INPUT_POWER_LOW_WARNING,
    .limits.ss_power_low_clear      = INPUT_POWER_LOW_CLEAR,
    .limits.magnet_current_error    = MAGNET_CURRENT_ERROR,
    .limits.vin_range.low           = INPUT_VOLTAGE_RANGE_LOW,
    .limits.vin_range.high          = INPUT_VOLTAGE_RANGE_HIGH,
    .limits.vin_range.margin        = INPUT_VOLTAGE_MARGIN,
    .serial_baud                    = CC_UCV_UART_BAUD_115200,
};


/*******************************************************************************
 * Client control params
*******************************************************************************/
cc_usv_t *cc_usv_get(int none)
{
    return &cc_usv;
}
safety_check_limits_t *client_limits_safety_limits_get(void)
{
    return &cc_usv.limits;
}

/**
 * Set lockout time before client error
 * @param time_us
 */
void cc_usv_lockout_time_set(int time_us)
{
    cc_usv.lockout_time = time_us;
}

/**
 * Number of tries when trying to communicate with keeper, anode, mag, ... 
 * with hsi pulse
 * @param misses_allowed
 */
void cc_usv_hci_misses_alowed_set(int misses_allowed)
{
    cc_usv.mci_hci_misses_allowed = misses_allowed;
}

/**
 * Get lockout time before client error
 */
int cc_usv_lockout_time_get(void)
{
    return cc_usv.lockout_time;
}

/**
 * Get lockout time before client error
 */
int cc_usv_lockout_save_time_get(void)
{
    return cc_usv.lockout_save_time;
}

/**
 * Number of tries when trying to communicate with keeper, anode, mag, ... 
 * with hsi pulse
 */
int cc_usv_hci_misses_alowed_get(void){
    return cc_usv.mci_hci_misses_allowed;
}

/*******************************************************************************
Safety limits checks 
*******************************************************************************/

/**
 * Set the keeper over voltage limit
 * @param mv millivolts
 */
void cc_usv_limit_keeper_overvoltage_set(keeper_septic_limits_t limits)
{
    cc_usv.limits.keeper_limits.critical = limits.critical;
    cc_usv.limits.keeper_limits.warn     = limits.warn;
    cc_usv.limits.keeper_limits.clear    = limits.clear;
}


/**
 * Set the Steady State power too high limit - The expected power for a given
 * setpoint, plus this wattage, will shutdown the thruster
 * @param mw milliwatts
 */
void cc_usv_limit_ss_power_limit_set(int mw)
{
    cc_usv.limits.ss_power_high_shutdown = mw;
}

/**
 * Set the Steady State power too high error - The expected power for a given
 * setpoint, plus this wattage will give an error message, but not shut down
 * the thruster
 * 
 * @param mw milliwatts
 */
void cc_usv_limit_ss_power_high_set(int mw)
{
    cc_usv.limits.ss_power_high_warning = mw;
}

/**
 * Set the Steady State power too low error - The expected power for a given
 * setpoint, minus this wattage will give an error message, but not shut down
 * the thruster
 * 
 * @param mw milliwatts
 */
void cc_usv_limit_ss_power_low_set(int mw)
{
    cc_usv.limits.ss_power_low_warning = mw;
}

/**
 * Set the magnet current error (+/-) 
 */
void cc_usv_limit_input_power_set(input_voltage_range_t r)
{
    cc_usv.limits.vin_range.low  = r.low;
    cc_usv.limits.vin_range.high = r.high;
}

/**
 * Set the magnet current error (+/-) 
 */
void cc_usv_limit_magnet_error_set(int err)
{
    cc_usv.limits.magnet_current_error = err;
}

/**
 * Get the keeper over voltage limit
 * @param mv millivolts
 */
keeper_septic_limits_t cc_usv_limit_keeper_overvoltage_get(void)
{
    return cc_usv.limits.keeper_limits;
}

/**
 * Get the Steady State power too high limit - The expected power for a given
 * setpoint, plus this wattage, will shutdown the thruster
 * @param mw milliwatts
 */
int cc_usv_limit_ss_power_limit_get(void)
{
    return cc_usv.limits.ss_power_high_shutdown;
}

/**
 * Get the Steady State power too high error - The expected power for a given
 * setpoint, plus this wattage will give an error message, but not shut down
 * the thruster
 * 
 * @param mw milliwatts
 */
int cc_usv_limit_ss_power_high_get(void)
{
    return cc_usv.limits.ss_power_high_shutdown;
}

/**
 * Get the Steady State power too low error - The expected power for a given
 * setpoint, minus this wattage will give an error message, but not shut down
 * the thruster
 * 
 * @param mw milliwatts
 */
int cc_usv_limit_ss_power_low_get(void)
{
    return cc_usv.limits.ss_power_high_shutdown;
}

/**
 * Get the max magnet current error (+/-)
 * @return magnet error in counts
 */
uint16_t cc_usv_limit_magnet_error_get(void)
{
    return (uint16_t)cc_usv.limits.magnet_current_error;
}

/**
 * Get the magnet current error (+/-) 
 * @return magnet error in milli-amps
 */
input_voltage_range_t cc_usv_limit_input_power_get(void)
{
    return cc_usv.limits.vin_range;
}

void cc_usv_serial_baud_set(uint8_t baud)
{
    cc_usv.serial_baud = baud;
}

uint8_t cc_usv_serial_baud_get(void)
{
    return cc_usv.serial_baud;
}