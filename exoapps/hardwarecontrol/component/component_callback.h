//
// Created by marvin on 4/21/23.
//

#ifndef MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_COMPONENT_COMPONENT_CALLBACK_H_
#define MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_COMPONENT_COMPONENT_CALLBACK_H_
#include "msg-handler/msg_handler.h"
#include "mcu_include.h"

int comp_cmd_cb(message_t *msg);
msg_callback_t *comp_get_hsi_cb(void);
void init_hsi_cb(void);

#endif //MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_COMPONENT_COMPONENT_CALLBACK_H_
