#ifndef _DMA_DRIVER_VORAGO_H
#define _DMA_DRIVER_VORAGO_H 1 
#include "stdint.h"

typedef enum{
    DMA_SERIAL_TX_CHANNEL,
    DMA_SERIAL_RX_CHANNEL
} dma_serial_channel_type_t;

typedef enum dma_increment{
    DMA_INCREMENT_BYTE,
    DMA_INCREMENT_HALFWORD,
    DMA_INCREMENT_WORD,
    DMA_INCREMENT_NO_INCREMENT,
    DMA_INCREMENT_EOL
} dma_increment_t;

typedef enum dma_cycle_control{
    DMA_CYCLE_CONTROL_STOP,
    DMA_CYCLE_CONTROL_BASIC,
    DMA_CYCLE_CONTROL_AUTO_REQUEST,
    DMA_CYCLE_CONTROL_PING_PONG,
    DMA_CYCLE_CONTROL_EOL
} dma_cycle_control_t;

typedef struct dma_channel_init{
    int irq_priority;
    volatile void* src_addr;
    dma_increment_t src_increment;
    volatile void* dest_addr;
    dma_increment_t dest_increment;
    uint32_t transaction_len;
    dma_cycle_control_t cycle_control;
} dma_channel_init_t;


/** 
 * create a DMA channel 
 *
 * @param dma_channel_init - init parameters for dma engine 
 * @param chan_type - channel type (uart TX or RX) configuration 
 * 
 * @return 0 on success or -1 on err
 */
int  dma_channel_create(const dma_channel_init_t* dma_channel_init, dma_serial_channel_type_t chan_type);


/** 
 * init DMA perihperal 
 *
 * @param void 
 * 
 * @return void
 */
void dma_init(void);


/** 
 * Reset channel parameters 
 *
 * @param handle dma handle to channel to reset parameters of 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_channel_reset(const int handle);


/** 
 * Change the length of the DMA transaction  
 *
 * @param handle dma handle to channel to reset parameters of 
 * @param tlen transaction length 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_transaction_length_set(const int handle, unsigned int tlen);


/** 
 * Change the source or destination address of the DMA transaction 
 *
 * @param handle dma handle to channel to reset parameters of 
 * @param src DMA source address 
 * @param dest DMA destination address
 *   
 * @return 0 on success or -1 on err
 */
int  dma_address_update(const int handle, void* src, void* dest);


/** 
 * Disable DMA interrupt  
 *
 * @param handle dma handle to channel to reset parameters of 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_interrupt_disable(const int handle);


/** 
 * Enable DMA interrupt  
 *
 * @param handle dma handle to channel to reset parameters of 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_interrupt_enable(const int handle);


/** 
 * Get the type of the DMA channel 
 *
 * @param handle dma handle to channel to reset parameters of 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_channel_type_get(const int handle);


/** 
 * Request a DMA transaction from SW 
 *
 * @param handle dma handle to channel to reset parameters of 
 *   
 * @return 0 on success or -1 on err
 */
int  dma_sw_request(const int handle);
#endif