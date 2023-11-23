#include "definitions.h"
#include "can/hal_can.h"


#if FREE_RTOS
TaskHandle_t task_handle;
static StackType_t task_stack[configMINIMAL_STACK_SIZE * 2];
static StaticTask_t task_buf;
#endif

static void can_task(void *pv);

int main(void)
{
    sys_init();

#if FREE_RTOS
    task_handle =
        xTaskCreateStatic(
            can_task,
            "comp",
            configMINIMAL_STACK_SIZE * 2,
            0,
            2,
            task_stack,
            &task_buf
                         );
    vTaskStartScheduler();
#elif
    can_task(NULL);
#endif

}

void can_task(void *pv)
{

    can_rx_buffer_t can_rx_buf[] = {
        {
            .rx_mob_count = 2,
            .filter_type = CAN_FILTER_ID,
            .filter_high = 0x81,
            .filter_low = 0x81
        },
        {
            .rx_mob_count = 1,
            .filter_type = CAN_FILTER_ID,
            .filter_high = 0x80,
            .filter_low = 0x80
        }
    };

    can_init_t can_ini = {
        .baud = CAN_BAUD_RATE_1000,
        .tx_mob_count = 10,
        .rx_buffers = can_rx_buf,
        .rx_buffer_len = 2,
        .irq_priority = 0
    };

    int can_handle = can_init(&can_ini);

    message_t msg = {0x1, {1, 2, 3, 4, 5, 6, 7}, 8};
    can_send(can_handle, &msg, 10000);

    uint32_t res;
    for(;;) {
        can_send(can_handle, &msg, 10000);
        for(int i = 0; i < 1000000; i++) {}
    }

}