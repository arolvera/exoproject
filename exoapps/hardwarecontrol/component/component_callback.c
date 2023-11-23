#include "component_callback.h"
#include "mcu_include.h"
#include "task-monitor/component_tasks.h"
#include "component_communication.h"
#include "operations.h"

static msg_callback_t hsi_cb = {
    .cb = comp_cmd_cb,
    .node = {
        .range_low = HSI_ID,
        .range_high = HSI_ID,
        .left = NULL,
        .right = NULL
    }
};

/**
 * Callback function for msg_handler
 * @param msg
 * @return
 */
int comp_cmd_cb(message_t *msg)
{
    uint8_t ops_id;
    if(msg->id == HSI_ID){
        //For each component enabled call its sync function
        const comp_task_list_t *temp = comp_task_list_get();
        for(int i = 0; i < temp->num_comp_tasks; i++){
            ops_id = commid_2_opsid(temp->comp_tasks[i]->comp_info.comm_id);
            if(ops_id != (uint8_t)-1){
                operations[ops_id].control_sync(msg);
            }
        }
    }else{
        ops_id = commid_2_opsid(msg->id & COMPONENT_COMM_ID_MASK);
        if(ops_id != (uint8_t)-1) {
            cc_command_processor(operations[ops_id].command_table_get(),
                                 msg->data,
                                 operations[ops_id].communication_id);
        }
    }
    return 0;
}

void init_hsi_cb(void)
{
    msg_handler_register_callback(&hsi_cb);
}
