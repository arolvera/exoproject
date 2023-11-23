#include "dma_driver_vorago.h"
#include "device.h"

#define DMA_CTRL_DEST_INC_Pos   30
#define DMA_CTRL_SRC_INC_Pos    26 
#define DMA_CTRL_R_POWER_Pos    14
#define DMA_CTRL_N_MINUS_1_Pos  4
#define DMA_CTRL_CYCLE_CTRL_Pos 0

#define DMA_CHANNEL_NUM 4

#define DMA_CTRL_BASE_PTR 0x20000000

typedef struct channel_ops{
	uint32_t src_addr;
	uint32_t dest_addr;
	uint32_t channel_config;
	uint32_t unused;
}channel_ops_t;


typedef struct dma_channel_control{
    channel_ops_t primary_channel[DMA_CHANNEL_NUM];
    channel_ops_t alternate_channel[DMA_CHANNEL_NUM];
} dma_channel_control_t;


typedef struct dma_ops{
    channel_ops_t* primary_channel;
    channel_ops_t* alternate_channel;
    uint32_t primary_channel_config;
    uint32_t alternate_channel_config;
    const uint32_t irq_num;
    dma_serial_channel_type_t chan_type;    
} dma_ops_t;

dma_ops_t dma_ops[DMA_CHANNEL_NUM] = {{.irq_num = DMA_DONE0_IRQn, .chan_type = -1},
                                             {.irq_num = DMA_DONE1_IRQn, .chan_type = -1}, 
                                             {.irq_num = DMA_DONE2_IRQn, .chan_type = -1}, 
                                             {.irq_num = DMA_DONE3_IRQn, .chan_type = -1}};

#define DMA_OPS_SIZE (sizeof(dma_ops)/sizeof(dma_ops[0]))

static dma_channel_control_t* dma_channel_control = (dma_channel_control_t*)(DMA_CTRL_BASE_PTR);

void dma_init(void)
{
    static int initd = 0;
    if(!initd){
        VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_DMA;

        VOR_SYSCONFIG->PERIPHERAL_RESET &= ~SYSCONFIG_PERIPHERAL_RESET_DMA_Msk;
        for (int cnt = 0; cnt < 7; cnt++){
        __NOP();
        }
        VOR_SYSCONFIG->PERIPHERAL_RESET |= SYSCONFIG_PERIPHERAL_RESET_DMA_Msk;
        for (int cnt = 0; cnt < 7; cnt++){
        __NOP();
        }
        initd = 1;
    }

    VOR_DMA->CTRL_BASE_PTR = DMA_CTRL_BASE_PTR;

    // Enable the DMA controller
    VOR_DMA->CFG |= DMA_CFG_MASTER_ENABLE_Msk;
}


int dma_channel_create(const dma_channel_init_t* dma_channel_init, dma_serial_channel_type_t chan_type)
{
    static int handle = 0;
    int err = 0;
    if(handle < DMA_CHANNEL_NUM){
        uint32_t dma_channel_config = 0;

        if(dma_channel_init->dest_increment < DMA_INCREMENT_EOL && 
           dma_channel_init->src_increment < DMA_INCREMENT_EOL){
            dma_channel_config |= dma_channel_init->dest_increment << DMA_CTRL_DEST_INC_Pos; 
            dma_channel_config |= dma_channel_init->src_increment << DMA_CTRL_SRC_INC_Pos;
        } else {
            err = -1;
        }

        if(!err && dma_channel_init->transaction_len < 1024){
            dma_channel_config |= ((dma_channel_init->transaction_len - 1) << DMA_CTRL_N_MINUS_1_Pos);
        } else {
            err = -1;
        }

        if(!err && dma_channel_init->cycle_control < DMA_CYCLE_CONTROL_EOL){
            dma_channel_config |= dma_channel_init->cycle_control << DMA_CTRL_CYCLE_CTRL_Pos;
        } else {
            err = -1;
        }

        if(!err){
            NVIC_SetPriority(dma_ops[handle].irq_num, dma_channel_init->irq_priority);
            NVIC_EnableIRQ(dma_ops[handle].irq_num); // Enable IRQ

            // Hard Coded for now.  If we need to adjust are R power the driver will probably need to be extended anyway
            dma_channel_config |= 4 << DMA_CTRL_R_POWER_Pos;

            dma_ops[handle].chan_type = chan_type;

            // Init primary channel
            dma_ops[handle].primary_channel = &dma_channel_control->primary_channel[handle];
            dma_ops[handle].primary_channel->channel_config = dma_channel_config;
            dma_ops[handle].primary_channel->dest_addr = (uintptr_t)dma_channel_init->dest_addr; 
            dma_ops[handle].primary_channel->src_addr  = (uintptr_t)dma_channel_init->src_addr; 
            dma_ops[handle].primary_channel_config = dma_channel_config;

            // Init alternate channel
            dma_ops[handle].alternate_channel = &dma_channel_control->alternate_channel[handle];
            dma_ops[handle].alternate_channel->channel_config = dma_channel_config;
            dma_ops[handle].alternate_channel->dest_addr = (uintptr_t)dma_channel_init->dest_addr; 
            dma_ops[handle].alternate_channel->src_addr  = (uintptr_t)dma_channel_init->src_addr; 
            dma_ops[handle].alternate_channel_config = dma_channel_config;            

            VOR_DMA->CHNL_ENABLE_SET |= (1 << handle);

        }
    } else {
        err = -1;
    }

    // if everything init'd ok return the handle, else return -1
    return err == 0 ? handle++ : err;
}


int dma_channel_reset(const int handle)
{
    int err = 0;
    if(handle < 0 || handle > DMA_CHANNEL_NUM){
        err = -1;
    } else {
        dma_ops[handle].primary_channel->channel_config   = dma_ops[handle].primary_channel_config;
        dma_ops[handle].alternate_channel->channel_config = dma_ops[handle].alternate_channel_config;
        VOR_DMA->CHNL_ENABLE_SET |= (1 << handle);
    }
    return err;
}

int dma_address_update(const int handle, void* src, void* dest)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        if(src != 0){
            dma_ops[handle].primary_channel->src_addr = (uintptr_t)src;
            dma_ops[handle].alternate_channel->src_addr = (uintptr_t)src;
        }

        if(dest != 0){
            dma_ops[handle].primary_channel->dest_addr = (uintptr_t)dest;
            dma_ops[handle].alternate_channel->dest_addr = (uintptr_t)dest;
        }
    } else {
        err = __LINE__;
    }

    return err;
}

int dma_transaction_length_set(const int handle, unsigned int tlen)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
    	dma_ops[handle].primary_channel->channel_config =  (dma_ops[handle].alternate_channel_config & ~(0x1FF << 4)) | ((tlen - 1) << 4);
	    dma_ops[handle].alternate_channel->channel_config = (dma_ops[handle].alternate_channel_config & ~(0x1FF << 4)) | ((tlen - 1) << 4);
    } else {
        err = __LINE__;
    }

    return err;
}


int dma_clear_pending_irq(const int handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        NVIC_ClearPendingIRQ(dma_ops[handle].irq_num);
    } else {
        err = __LINE__;
    }

    return err;
}


int dma_interrupt_disable(const int handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        NVIC_DisableIRQ(dma_ops[handle].irq_num);
    } else {
        err = __LINE__;
    }

    return err;
}


int dma_interrupt_enable(const int handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        NVIC_EnableIRQ(dma_ops[handle].irq_num);
    } else {
        err = __LINE__;
    }

    return err;
}


int dma_channel_type_get(const int handle)
{
    int err = 0;
    dma_serial_channel_type_t chan_type;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        chan_type = dma_ops[handle].chan_type;
    } else {
        err = -1;
    }

    return err == 0 ? chan_type : err;
}


int dma_sw_request(const int handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)DMA_OPS_SIZE){
        VOR_DMA->CHNL_SW_REQUEST |= 1 << handle;
    } else {
        err = __LINE__;
    }

    return err;
}