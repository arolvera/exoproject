#include "mcan_task.h"


uint32_t can_send(const int handle, message_t* msg, const uint32_t timeout)
{
    int err = 0;

    if(msg == 0){
        err = __LINE__;
    }

    if(!err) {
        if(handle == MCAN_CLIENT_HANDLE) {
            mcan_send_msg_client(msg->id, msg->data, msg->dlc);
        } else if(handle == MCAN_EXTERN_HANDLE) {
            mcan_send_msg_extern(msg->id, msg->data, msg->dlc);
        } else {
            err = __LINE__;
        }
    }
    return err;
}
