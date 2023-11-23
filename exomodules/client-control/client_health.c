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
#include "client_health.h"
#include "client_control.h"
#include "client_error_details.h"
#include "client_p.h"
#include "control_anode.h"
#include "error/error_handler.h"
#include "health/health.h"
#include "keeper/control_keeper.h"
#include "mcu_include.h"
#include "setpoint/control_setpoint.h"
#include "sys/sys_timers.h"
#include "throttle/control_throttle.h"
#include "trace/trace.h"
#include "user-setting-values/client_control_usv.h"

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_HEALTH

#define KEEPER_REV_CHECK_ENABLE 0

/* How many consecutive times an MCU can miss an HSI update before taking action */
#define CONSECUTIVE_MCI_HSI_MISSES_ALLOWED 4

/**
 * Check Input Power against expected setpoint power
 *
 * NOTE: only call this when in Steady State and NOT throttling
 *
 * @return 0 if in range -1 otherwise
 */
int client_setpoint_power_check(void)
{
    int err = 0;
    int output_p = 0;

    int setpoint = 0;

    safety_check_limits_t *limits = client_limits_safety_limits_get();

    float f = 0;

    setpoint = ctrl_sequence_setpoint_get();

    err = thrust_power_get(setpoint, &f);
    if(!err) {
        int sp_val = (f * 1000);
        int high_shutdown   = sp_val + limits->ss_power_high_shutdown;
        int high_warn_clear = sp_val + limits->ss_power_high_clear;
        int high_warning    = sp_val + limits->ss_power_high_warning;
        int low_warning     = sp_val - limits->ss_power_low_warning;
        int low_warn_clear  = sp_val - limits->ss_power_low_clear;


        output_p = ctrl_anode_p_out_get();

        /*
         * capture error handling specific details
         */
        client_control_specific_detail_t c = {0};
        c.power_level.power_level = output_p;
        c.power_level.setpoint = setpoint + 1; // Index at 1 for the outside world
        unsigned int power_error = 0;

        int power_high_fault = eh_fault_status_get(ERROR_CODE_THRUSTER_POWER_HIGH_FAULT);
        int power_high_warn  = eh_fault_status_get(ERROR_CODE_THRUSTER_POWER_HIGH_WARN);
        int power_low_warn   = eh_fault_status_get(ERROR_CODE_THRUSTER_POWER_LOW_WARN);

        if(output_p > high_shutdown) {
            err = __LINE__;
            if(!power_high_fault){
                TraceE2(TrcMsgMcuCtl, "Input power limit exceeded. ip:%d limit:%d sp[%d]:%d",
                        output_p, high_shutdown, setpoint+1, sp_val,0,0);
                power_error = ERROR_CODE_THRUSTER_POWER_HIGH_FAULT;
            }
            err = __LINE__;

        } else if(output_p > high_warning) {
            if(!power_high_warn){
                TraceE2(TrcMsgMcuCtl, "Input power higher than expected. ip:%d: limit:%d sp[%d]:%d",
                        output_p, high_warning, setpoint+1, sp_val,0,0);
                power_error = ERROR_CODE_THRUSTER_POWER_HIGH_WARN;
            }
            err = __LINE__;

        } else if(output_p < low_warning) {
            if(!power_low_warn){
                TraceE2(TrcMsgMcuCtl, "Input power lower than expected. ip:%d limit:%d sp[%d]:%d",
                        output_p, low_warning, setpoint+1, sp_val,0,0);
                power_error = ERROR_CODE_THRUSTER_POWER_LOW_WARN;
            }
            err = __LINE__;
        }

        if(power_error){
            ERROR_SET(TC_EMCY_REG_CURRENT, power_error, &c);
        }

        if(output_p < high_warning){
            ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_HIGH_FAULT);
        }

        if(output_p < high_warn_clear){
            ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_HIGH_WARN);
        }

        if(output_p > low_warn_clear){
            ERROR_CLEAR(TC_EMCY_REG_CURRENT, ERROR_CODE_THRUSTER_POWER_LOW_WARN);
        }

#if __DEBUG
        static SYSTMR ss_power_debug_timer = 0;
        if(ss_power_debug_timer == 0){
            TraceDbg(TrcMsgMcuCtl, "op:%d sp[%d]:%d", output_p, setpoint+1, sp_val, 0,0,0);
        }
        if(ss_power_debug_timer == 0) {
            sys_timer_start(1 SECONDS, &ss_power_debug_timer);
        }

#endif // DEBUG
    }
    return err;
}

int client_vin_check(void)
{
    int err = 0;
    int fault_set = 0;
    float input_v = 0;
    client_control_specific_detail_t d = {0};

    input_voltage_range_t r =  cc_usv_limit_input_power_get();

    input_v = ctrl_keeper_v_in_get();
    d.voltage = input_v * 1000;

    fault_set = eh_fault_status_get(ERROR_CODE_HKM_UNDER_VOLTAGE);
    if(input_v < r.low - r.margin) {
        err = ERROR_CODE_HKM_UNDER_VOLTAGE;
    }
    if(!err) {
        fault_set = eh_fault_status_get(ERROR_CODE_HKM_OVER_VOLTAGE);
        if(input_v > r.high + r.margin) {
            err = ERROR_CODE_HKM_OVER_VOLTAGE;
        }
    }
    if(err) {
        ERROR_SET(TC_EMCY_REG_MANUFACTURER, err, &d);
        if(!fault_set) {
            TraceE2(TrcMsgErr2, "Input Voltage out of range. vin:%d l:%d h:%d m:%d",
                    d.voltage, r.low * 1000, r.high * 1000, r.margin,0,0);
        }
    }
    if(!err) {
        if(input_v > r.low) {
            ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_HKM_UNDER_VOLTAGE);
        }
        if(input_v < r.high) {
            ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_HKM_OVER_VOLTAGE);
        }
    }


    return err;
}

