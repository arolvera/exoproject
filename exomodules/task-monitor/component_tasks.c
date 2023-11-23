#include "component_tasks.h"
#include "task_priority.h"

int comp_task_can_init(QueueHandle_t *rx_q_handle)
{
    comp_can_init_t *cp = comp_task_group_get_can_rx_bufs();
    int handle = -1;

    if(cp) {
        //Setup can init var
        can_init_t ci =
            {
                .baud = CAN_BAUD_RATE_1000,
                .rx_buffers = cp->rx_bufs,
                .rx_buffer_len = cp->num_rx_bufs,
                .task_priority = MSG_HANDLER_TASK_PRIO,
                .tx_mob_count = cp->tx_mob_count,
                .rx_q_handle = rx_q_handle,
            };
        handle = can_init(&ci);
    }
    return handle;
}
