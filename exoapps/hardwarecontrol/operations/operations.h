#ifndef MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_INCLUDE_OPERATIONS_DEF_H_
#define MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_INCLUDE_OPERATIONS_DEF_H_
#include <stdbool.h>
#include "mcu_include.h"
#include "ext_decl_define.h"
#include "msg_callback.h"
#ifndef SINGLE_COMPONENT_BUILD
#include "msg-handler/msg_handler.h"
#endif //SINGLE_COMPONENT_BUILD

// Different states
typedef enum
{
  INIT_STATE,
  OFF_STATE,
  STARTUP_STATE,
  ON_STATE,
  ERROR_STATE,
  NUM_VALID_STATES,
} states_t;

// these IDs are just placeholders
typedef enum {
  KEEPER_ID = 0,
  ANODE_ID = 1,
  MAGNET_ID = 2,
  VALVE_ID = 3
  //Append mcus to the end
} mcu_ids_t;

// Typedef of shared function pointers
typedef void (*state_handler_t)(void);
typedef command_table_t* (*command_table_get_t)(void);
typedef states_t (*current_state_get_t)(void);
typedef bool (*control_flag_check_t)(void);
typedef void (*control_flag_reset_t)(void);
typedef void (*data_get_t)(void);
typedef int (*control_sync_t)(message_t *msg);  //Component sync

// Data structure for shared global variables
typedef struct
{
  states_t current_state;
  uint8_t error_code;
  uint16_t error_adc;
} component_control_t;

typedef struct operations_t_ {
  state_handler_t state_handler[NUM_VALID_STATES];
  command_table_get_t command_table_get;
  current_state_get_t current_state_get;
  control_flag_check_t control_flag_check;
  control_flag_reset_t control_flag_reset;
  control_sync_t control_sync;
  data_get_t data_get;
  uint16_t communication_id;
#ifndef SINGLE_COMPONENT_BUILD
  msg_callback_t *cb;
#endif //SINGLE_COMPONENT_BUILD
}operations_t;

//Converts comm_id to operations array id/pos. returns -1 if comm_id does not match component
uint8_t commid_2_opsid(uint8_t comm_id);
uint8_t opsid_2_commid(uint8_t opsid);

//Operations array
EXT_DECL operations_t operations[];

#endif //MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_HARDWARECONTROL_INCLUDE_OPERATIONS_DEF_H_
