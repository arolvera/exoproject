#include "definitions.h"
#include "thruster_control.h"
#include "thruster-start/control_thruster_start.h"
#include "trace/trace.h"
#include "throttle/control_throttle.h"
#include "canopen/co_task.h"
#include "setpoint/control_setpoint.h"

#define TS_MONITOR_WAIT_MS  250
#define OP_MONITOR_WAIT_MS 1000

#define cond_TASK_PRIORITY   (tskIDLE_PRIORITY)
#define cond_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
static TaskHandle_t auto_start_task_handle;
static StackType_t  autostart_task_stack[cond_TASK_STACK_SIZE];
static StaticTask_t autostart_TaskBuffer;

static volatile bool task_running = false;
static uint32_t throttle_setpoint = 0;

static void ctrl_as_wait_transition_done(void)
{
    /* Initialize xNextWakeTime - this only needs to be done once. */
    TickType_t xNextWakeTime = xTaskGetTickCount();
    do {
        vTaskDelayUntil(&xNextWakeTime, (TS_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
    } while(ctrl_ts_in_transition() != 0);
    // Take it slow.  No rush here
    vTaskDelayUntil(&xNextWakeTime, (TS_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
}

static int ctrl_as_operational(void)
{
    TickType_t xNextWakeTime = xTaskGetTickCount();
    if(Thruster_Control_State == TCS_CO_PREOP) {
        canopen_task_nmt_op();
        do {
            vTaskDelayUntil(&xNextWakeTime, (OP_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
        } while(Thruster_Control_State == TCS_TRANISTION_STANDBY);
    }
    return (Thruster_Control_State == TCS_STANDBY) ? 0:__LINE__;
}

static int ctrl_as_fire(uint32_t setpoint)
{
    int err = 0;
    
    err = ctrl_as_operational();
    if(!err) {
        err = ctrl_ts_ready_mode_run();
    }
    if(!err) {
        ctrl_as_wait_transition_done();
        err = ctrl_ts_steady_state_run(0);
    }
    if(!err) {
        ctrl_as_wait_transition_done();
        err = (THRUSTER_IN_STEADY_STATE() ? 0:__LINE__);
    }
    if(!err) {
        err = ctrl_throttle_setpoint_set(setpoint);
    }
    return err;
}

static void ctrl_autostart_task(void *pvParameters)
{
    uint32_t setpoint = *((uint32_t *) pvParameters);
    TraceInfo(TrcMsgSeq, "Autostart task setpoint:%d", setpoint,0,0,0,0,0);
    
    int err = ctrl_as_fire(setpoint);
    
    task_running = false;
    TraceInfo(TrcMsgSeq, "Autostart task delete. err:%d", err,0,0,0,0,0);
    vTaskDelete(NULL);
}

void ctrl_autostop(void)
{
    if(task_running) {
        task_running = false;
        vTaskDelete(auto_start_task_handle);
    }
}

int ctrl_autostart(uint32_t setpoint)
{
    int err = 0;
    uint32_t max = thrust_table_max_valid();
    
    if(task_running) {
        err = __LINE__;
    }
    if(!err && setpoint > max) {
        err = __LINE__;
    }
    if(!err &&
       Thruster_Control_State != TCS_CO_PREOP &&
       Thruster_Control_State != TCS_STANDBY) {
        err = __LINE__;
    }
    if(!err) {
        throttle_setpoint = setpoint;
        task_running = true;
        auto_start_task_handle = xTaskCreateStatic(ctrl_autostart_task, "Autostart Task",
                cond_TASK_STACK_SIZE, &throttle_setpoint, cond_TASK_PRIORITY,
                autostart_task_stack, &autostart_TaskBuffer);
    }
    return err;
}
