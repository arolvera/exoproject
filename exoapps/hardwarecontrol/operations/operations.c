#include "operations.h"

uint8_t commid_2_opsid(uint8_t comm_id)
{
    switch(comm_id) {
        case COMM_ID_KEEPER:
            return KEEPER_ID;
        case COMM_ID_ANODE:
            return ANODE_ID;
        case COMM_ID_MAGNET:
            return MAGNET_ID;
        case COMM_ID_VALVE:
            return VALVE_ID;
        default:
            return -1;
    }
}

uint8_t opsid_2_commid(uint8_t opsid)
{
    switch(opsid) {
        case KEEPER_ID:
            return COMM_ID_KEEPER;
        case ANODE_ID:
            return COMM_ID_ANODE;
        case MAGNET_ID:
            return COMM_ID_MAGNET;
        case VALVE_ID:
            return COMM_ID_VALVE;
        default:
            return -1;
    }
}
