/**
 * @file    client_power_halo12.c
 *
 * @brief   Implementation for halo12 specific client power control.
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

#include "definitions.h"
#include "client-control/client_control.h"
#include "task-monitor/component_tasks.h"
#include "client_power.h"
#include "task-monitor/component_tasks.h"
#include "device.h"
#include "mcu_include.h"
#include "msg-handler/msg_handler.h"

#define POWER_ON_SOFT_START_DELAY 100
#define SOFTSTART_DELAY() (vTaskDelay(POWER_ON_SOFT_START_DELAY / portTICK_RATE_MS))
#define POWER_ON_DELAY() (vTaskDelay(1 / portTICK_RATE_MS))

static void cp_power_mag_valv_anode(bool on)
{
    //ACP shares the same gpio power line. Only need to power up/down mvcp
    if(on) {
        comp_power(MVCP_RESID, on);
        POWER_ON_DELAY();
    } else {
        client_comp_powered_down(COMPONENT_MAGNET, false);
        client_comp_powered_down(COMPONENT_VALVES, false);
        client_comp_powered_down(COMPONENT_ANODE, false);
        comp_power(MVCP_RESID, on);
    }
}

static void cp_power_keeper(bool on)
{
    comp_power(ECPK_RESID, on);     // starts the keeper task
    POWER_ON_DELAY();
    if(!on){
        client_comp_powered_down(COMPONENT_KEEPER, false);     // stops the keeper task
    }
}

void cp_power_set(bool power_on)
{
    cp_power_keeper(power_on);
    cp_power_mag_valv_anode(power_on);
}

int cp_client_reset(int client_id)
{
    int err = 0;
    message_t rst_msg = {.id = COMMAND_PARAMETERS_ID_BASE | client_id,
                         .dlc = 8,
                         .data[0] = 0xFF};
    if (client_id < 0) {
        err = __LINE__;
    } else {
        send_msg(rst_msg.id,
                 rst_msg.data,
                 rst_msg.dlc,
                 0);
    }
    return err;
}

void cp_reset_all(void)
{
    // delete the keeper task
    comp_power(ECPK_RESID, false);
    POWER_ON_DELAY();

    // update the state and clear the boot status
    client_comp_powered_down(COMPONENT_KEEPER, true);
    client_comp_powered_down(COMPONENT_MAGNET, true);
    client_comp_powered_down(COMPONENT_VALVES, true);
    client_comp_powered_down(COMPONENT_ANODE, true);

    // send the reset command to the clients
    cp_client_reset(COMM_ID_ANODE);

    cp_client_reset(COMM_ID_MAGNET);

    // restart the keeper task
    comp_power(ECPK_RESID, true);     // starts the keeper task
    POWER_ON_DELAY();
}
