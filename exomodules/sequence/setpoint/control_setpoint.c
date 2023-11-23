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

/* 
 * File:   
 * Author: jmeyers
 * 
 * @Company
 * Exoterra
 * 
 * @File Name
 * control_thrust.c
 * 
 * @Summary
 * This module does the work of setting/getting/throttling to thrust setpoints.
 * 
 * @Description
 * see header file
 * 
 * Created on July 29, 2021, 1:30 PM
 */

#include "control_setpoint.h"

/**
 * Return the number of valid entries in the table.
 * The first entry in the table with anode voltage a zero is the end of the 
 * list
 * @return number of valid entries 
 */
uint32_t thrust_table_max_valid(void)
{
    uint32_t count = 0;
    float anode_v = (float)99.99;
    /* Not an efficient search but the number of entries is only four at the
     * time of this writing so....  */
    for(int i = 0; i < THRUST_TABLE_SIZE && anode_v > 0.0; i++) {
        anode_v = throttle_table[i].anode_v;
        if(anode_v > 0.0) {
            count++;
        }
    }
    return count;
}

int thrust_table_entry_get(uint32_t setpoint, thrust_data_t **data)
{
    int err = 0;
    if(setpoint >= THRUST_TABLE_SIZE) {
        err  = -1;
    } else {
        *data = &throttle_table[setpoint];
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_cathode_lf_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].cathode_lf;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_cathode_lf_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].cathode_lf = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_flow_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].anode_flow;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_flow_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].anode_flow = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_v_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].anode_v;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_v_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].anode_v = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_i_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].anode_i;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_anode_i_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].anode_i = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_magnet_i_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].magnet_i;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_magnet_i_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].magnet_i = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_magnet_ratio_setpoint_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].magnet_ratio;
    }
    return err;
}

/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_magnet_ratio_setpoint_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].magnet_ratio = value;
    }
    return err;
}

/**
 * Get the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_millinewtons_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].thrust;
    }
    return err;
}


/**
 * Set the setpoint for the given thrust setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_millinewtons_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].thrust = value;
    }
    return err;
}

/**
 * Get the HARD or GLOW start setting for the given setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_start_method_get(uint32_t setpoint, start_method_t *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].start_method;
    }
    return err;
}


/**
 * Specify HARD or GLOW start for the given setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_start_method_set(uint32_t setpoint, start_method_t value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE && value < START_METHOD_COUNT) {
        err = 0;
        throttle_table[setpoint].start_method = value;
    }
    return err;
}


/**
 * Specify power setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the power value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_power_set(uint32_t setpoint, float value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].power = value;
    }
    return err;
}


/**
 * Get power setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the power value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_power_get(uint32_t setpoint, float *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].power;
    }
    return err;
}



/**
 * Specify HARD or GLOW start for the given setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_timeout_set(uint32_t setpoint, uint32_t value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].timeout = value;
    }
    return err;
}


/**
 * Get the HARD or GLOW start setting for the given setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_timeout_get(uint32_t setpoint, uint32_t *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].timeout;
    }
    return err;
}


/**
 * Set the highf low setpoint
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_hf_start_setpoint_set(uint32_t setpoint, uint32_t value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        throttle_table[setpoint].hf_start_setpoint = value;
    }
    return err;
}


/**
 * Get the high flow setpoint 
 * @param setpoint thrust setpoint (which row in the throttle_table)
 * @param value pointer to store the thrust value
 * @return 0 on success -1 on error (setpoint out of range)
 */
int thrust_hf_start_setpoint_get(uint32_t setpoint, uint32_t *value)
{
    int err = -1;
    
    if(setpoint < THRUST_TABLE_SIZE) {
        err = 0;
        *value = throttle_table[setpoint].hf_start_setpoint;
    }
    return err;
}





