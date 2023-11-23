#include "control_condition_object.h"
#include "conditioning//control_condition.h"       // Condition routines
#include "user_object_od_indexes.h"

static CO_ERR CtrlCondWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

/** 
 * 0x2300 - Magnet Control User Object
 */
CO_OBJ_TYPE CtrlCondType = {
    0,   /* type function to get object size      */
    0,            /* type function to control type object  */
    0,   /* type function to read object content  */
    CtrlCondWrite,  /* type function to write object content */
};
#define CO_CtrlCond  ((CO_OBJ_TYPE*)&CtrlCondType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    CONDITION_STAT_COUNT        = 0,
    CONDITION_STAT_CURRENT_STEP = 1, /* Current (last ran) conditioning step */
    CONDITION_STAT_STAT_BASE    = 2, /* First Condition Stat sub-index       */
} OD_CONTROL_CONDITION_SUBIDX;

enum {
    CONDITION_STAT_CLEAR_COUNT = 0,
    CONDITION_STAT_CLEAR_CLEAR = 1,
    CONDITION_STAT_CLEAR_EOL   = 2,
};

enum {
    CONDITION_UCV_COUNT        = 0,
};

//Magic num to confirm cond stat clear
#define CTRLCONDWR_MAGIC_NUM 0x63637772 //ccwr

static CO_ERR CtrlCondWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int err = -1;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint16_t idx = CO_GET_IDX(obj->Key);
    uint32_t val =  (size == 0) ? 0xFFFFFFFF : ((uint32_t*)buf)[0];
    uint32_t clrconfirm = CTRLCONDWR_MAGIC_NUM; 
    if(idx == OD_INDEX_CONDITION_STAT_CLEAR){
        switch(subidx){
            case CONDITION_STAT_CLEAR_CLEAR:
                if(clrconfirm == val){
                    err = ctrl_condition_status_clear();
                }
                break;
            default:
                err = -1;                
        }
    }
    
    return err ? CO_ERR_TYPE_WR : CO_ERR_NONE;
}

void CtrlCondOD(OD_DYN *self)
{
    condition_steps_stat_t stats;
    int err = ctrl_condition_stats_get(&stats);
    uint32_t co_offset = 0;
    uint32_t total_subs;
    if(!err) {
        total_subs = (stats.count * 3) + CONDITION_STAT_STAT_BASE;
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT, CONDITION_STAT_COUNT,        CO_UNSIGNED8  | CO_OBJ_D__R_), 0, total_subs);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT, CONDITION_STAT_CURRENT_STEP, CO_UNSIGNED32 | CO_OBJ____R_), 0, (uintptr_t) stats.step);
        for(uint32_t i = 0; i < stats.count; i++) {
            condition_stat_t *p = &stats.step_stats[i];
            co_offset =(i*3) + CONDITION_STAT_STAT_BASE;
            ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT, co_offset++, CO_UNSIGNED32 |CO_OBJ____R_), 0, (uintptr_t) &p->seq_stat_cond);
            ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT, co_offset++, CO_UNSIGNED32 |CO_OBJ____R_), 0, (uintptr_t) &p->elapsed_ms);
            ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT, co_offset++, CO_UNSIGNED32 |CO_OBJ____R_), 0, (uintptr_t) &p->monitor_err);
        }
    }
    
    //Clear command
    ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT_CLEAR, CONDITION_STAT_CLEAR_COUNT, CO_UNSIGNED8 | CO_OBJ_D__R_), 0, CONDITION_STAT_CLEAR_EOL - 1);
    ODAdd(self, CO_KEY(OD_INDEX_CONDITION_STAT_CLEAR, CONDITION_STAT_CLEAR_CLEAR, CO_UNSIGNED32 |CO_OBJ_____W), CO_CtrlCond, (uintptr_t) &CtrlCondType);

    uint16_t num_seq = ctrl_condition_num_sequence_get();
    total_subs = num_seq * 7;
    ctrl_cond_seq_ucv_t *seq_ucv;
    co_offset = 1;
    ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, CONDITION_UCV_COUNT, CO_UNSIGNED8 |CO_OBJ_D__R_), 0, total_subs);
    for(uint32_t i = 0; i < num_seq; i++ ){
	seq_ucv = ctrl_condition_seq_ucv_get(i);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED32 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->monitor_ms);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.adjust_limit_lower);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.adjust_limit_upper);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.current_limit);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.max_limit);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.power_limit);
        ODAdd(self, CO_KEY(OD_INDEX_CONDITION_LIMIT, co_offset++, CO_UNSIGNED16 |CO_OBJ____RW), 0, (uintptr_t) &seq_ucv->limits.voltage_limit);
    }
}