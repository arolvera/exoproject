#ifndef _IRQ_ROUTER_DRIVER_VORAGO_H
#define _IRQ_ROUTER_DRIVER_VORAGO_H 1

#include <stdint.h>

#define IRQ_ROUTER_TRIGGER_UART0_TX 8
#define IRQ_ROUTER_TRIGGER_UART0_RX 9
#define IRQ_ROUTER_TRIGGER_UART1_TX 10 
#define IRQ_ROUTER_TRIGGER_UART1_RX 11
#define IRQ_ROUTER_TRIGGER_UART2_TX 12
#define IRQ_ROUTER_TRIGGER_UART2_RX 13


int irq_router_dma_channel_trigger_set(const uint32_t handle, const uint32_t dma_channel_trigger);
void irq_router_init(void);

#endif