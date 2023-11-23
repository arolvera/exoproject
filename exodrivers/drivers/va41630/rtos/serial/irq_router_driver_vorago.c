#include <stdbool.h>
#include "irq_router_driver_vorago.h"
#include "device.h"

#define IRQ_ROUTER_NUM_TRIGGERS 71


typedef struct irq_router_ops {
    volatile uint32_t* dma_sel;
    bool channel_routed;
} irq_router_ops_t;

#define NUM_DMA_CHANNELS 4

static irq_router_ops_t irq_router_ops[NUM_DMA_CHANNELS] = {{.dma_sel = (volatile uint32_t*)&VOR_IRQ_ROUTER->DMASEL0, .channel_routed = false}, 
                                                            {.dma_sel = (volatile uint32_t*)&VOR_IRQ_ROUTER->DMASEL1, .channel_routed = false}, 
                                                            {.dma_sel = (volatile uint32_t*)&VOR_IRQ_ROUTER->DMASEL2, .channel_routed = false}, 
                                                            {.dma_sel = (volatile uint32_t*)&VOR_IRQ_ROUTER->DMASEL3, .channel_routed = false}};



/*
 * Enable the perihperal's clock, but DO NOT RESET IF USING FREERTOS
 */
void irq_router_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_IRQ;
}


/*
 * Set the event that will trigger a DMA transaction
 */
int irq_router_dma_channel_trigger_set(const uint32_t handle, const uint32_t dma_channel_trigger)
{ 
    int irq_router_channel_num = handle;
    int err = 0;

    if(irq_router_channel_num < NUM_DMA_CHANNELS && dma_channel_trigger < IRQ_ROUTER_NUM_TRIGGERS){
        irq_router_ops_t* irq_ops = &irq_router_ops[irq_router_channel_num];
        *irq_ops->dma_sel = (IRQ_ROUTER_DMASEL0_DMASEL_Msk & dma_channel_trigger);
        irq_ops->channel_routed = true;
    } else {
        err = -1;
    }

    return err == 0 ? irq_router_channel_num++ : err;
}