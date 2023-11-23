#include "uart_driver_vorago.h"
#include "device.h"
#ifdef FREE_RTOS
#include "serial/driver_serial_va41630_rtos.h"
#endif

/* #define for readability purposes */
#define IOCONFIG_PORTx_FUNSEL_Pos IOCONFIG_PORTA_FUNSEL_Pos

#define UART_CALC_CLOCKSCALE(_scc,_baud) ((_scc / (_baud * 16)) << \
                   UART_CLKSCALE_INT_Pos) |  \
                   (((((_scc % (_baud * 16)) * \
                   64 + (_baud * 8)) / \
                   (_baud * 16))) << \
                   UART_CLKSCALE_FRAC_Pos)


typedef struct uart_ops{
    const uart_if_id_t uart_if_id;
    volatile uint32_t* uart_data;
    const int clock_enable;
    const int peripheral_reset;
    VOR_UART_Type* const uart_ctrl;
    const uint32_t rx_irq_num;
    const uint32_t tx_irq_num;
    bool uart_initd;
    volatile uint32_t* rx_port_setting;
    volatile uint32_t* tx_port_setting;
    int port_setting_value;
    volatile const uint32_t* uart_tx_status;
} uart_ops_t;

#define UART_0_PORT_G_FUNSEL 1
#define UART_1_PORT_B_FUNSEL 3
#define UART_2_PORT_F_FUNSEL 1

static uart_ops_t uart_ops[] = {{.uart_if_id = UART_IF_ID_0,       .uart_data = &VOR_UART0->DATA,
                                         .clock_enable = CLK_ENABLE_UART0, .peripheral_reset = SYSCONFIG_PERIPHERAL_RESET_UART0_Msk, 
                                         .uart_ctrl = VOR_UART0,           .rx_irq_num = UART0_RX_IRQn,  
                                         .tx_irq_num = UART0_TX_IRQn,      .uart_initd = false, 
                                         // UART 2 is on port G pins 0 and 1
                                         .tx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTG[0], 
                                         .rx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTG[1], 
                                         .port_setting_value = UART_0_PORT_G_FUNSEL,
                                         .uart_tx_status = &VOR_UART0->TXSTATUS},

                                        {.uart_if_id = UART_IF_ID_1,       .uart_data = (volatile uint32_t*)&VOR_UART1->DATA,
                                         .clock_enable = CLK_ENABLE_UART1, .peripheral_reset = SYSCONFIG_PERIPHERAL_RESET_UART1_Msk, 
                                         .uart_ctrl = VOR_UART1,           .rx_irq_num = UART1_RX_IRQn,
                                         .tx_irq_num = UART1_TX_IRQn,      .uart_initd = false,
                                         // UART 2 is on port B pins 14 and 15 
                                         .tx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTB[14], 
                                         .rx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTB[15], 
                                         .port_setting_value = UART_1_PORT_B_FUNSEL,
                                         .uart_tx_status = &VOR_UART1->TXSTATUS},

                                        {.uart_if_id = UART_IF_ID_2,       .uart_data = (volatile uint32_t*)&VOR_UART2->DATA,
                                         .clock_enable = CLK_ENABLE_UART2, .peripheral_reset = SYSCONFIG_PERIPHERAL_RESET_UART2_Msk, 
                                         .uart_ctrl = VOR_UART2,           .rx_irq_num = UART2_RX_IRQn, 
                                         .tx_irq_num = UART2_TX_IRQn,      .uart_initd = false,
                                         // UART 2 is on port F pins 8 and 9
                                         .tx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTF[8], 
                                         .rx_port_setting = (volatile uint32_t*)&VOR_IOCONFIG->PORTF[9], 
                                         .port_setting_value = UART_2_PORT_F_FUNSEL,
                                         .uart_tx_status = &VOR_UART2->TXSTATUS}};


#define UART_OPS_SIZE (sizeof(uart_ops) / sizeof(uart_ops[0]))



// IRQs used to trigger DMAs when used by the serial driver
void UART0_TX_IRQHandler(void)
{
}

void UART0_RX_IRQHandler(void)
{
    /* Reset interrupt level for resync purposes */
	VOR_UART0->RXFIFOIRQTRG = 0xD;
}

void UART1_TX_IRQHandler(void)
{
}

void UART1_RX_IRQHandler(void)
{
    /* Reset interrupt level for resync purposes */
	VOR_UART1->RXFIFOIRQTRG = 0xD;
}

void UART2_TX_IRQHandler(void)
{
}

void UART2_RX_IRQHandler(void)
{
    /* Reset interrupt level for resync purposes */
 	VOR_UART2->RXFIFOIRQTRG = 0xD;
}


int uart_tx_status_get(int uart_handle)
{
    return (*(uart_ops[uart_handle].uart_tx_status) & (UART_TXSTATUS_TXBUSY_Msk | UART_TXSTATUS_WRBUSY_Msk));
}

int uart_init(const uart_init_t* ui)
{
    int err = 0;
    unsigned int uart_if_id = ui->uart_if_id;
    uart_ops_t* uart_op = 0;

    if(uart_if_id >= UART_IF_ID_EOL){
        err = -1;
    } else {
        uart_op = &uart_ops[uart_if_id];        

        VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= uart_op->clock_enable;
        VOR_SYSCONFIG->PERIPHERAL_RESET &= ~uart_op->peripheral_reset;
        __NOP();
        __NOP();
        VOR_SYSCONFIG->PERIPHERAL_RESET |= uart_op->peripheral_reset;

        /* Set up gpio port to be uart */
        *uart_op->rx_port_setting = uart_op->port_setting_value << IOCONFIG_PORTx_FUNSEL_Pos;
        *uart_op->tx_port_setting = uart_op->port_setting_value << IOCONFIG_PORTx_FUNSEL_Pos;

        unsigned int baud = ui->baud;
        
        if(baud > 0){
            unsigned int clk_div = 4;
            
            /* Uart 2 is clocked differently */
            if(uart_if_id == UART_IF_ID_2){
                clk_div = 2;
            }

            /* NOTE: This init assumes a lot:
                no parity
                1 stop bit
                8 bit word
                no loopback
                no flowcontrol
                */
            uint32_t uart_ctrl = 0; 
            uart_op->uart_ctrl->CLKSCALE = UART_CALC_CLOCKSCALE(SystemCoreClock/clk_div, baud);
            uart_ctrl = 3 << UART_CTRL_WORDSIZE_Pos;
            uart_op->uart_ctrl->CTRL = 0x200 | uart_ctrl;
        
            if(ui->rx_irq_enable){
                uart_op->uart_ctrl->IRQ_ENB |= UART_IRQ_ENB_IRQ_RX_Msk;
                uart_op->uart_ctrl->RXFIFOIRQTRG = ui->rx_interrupt_level;
                NVIC_EnableIRQ(uart_op->rx_irq_num);
                NVIC_SetPriority(uart_op->rx_irq_num, ui->rx_irq_priority);
            }

            if(ui->tx_irq_enable){
                uart_op->uart_ctrl->IRQ_ENB |= UART_IRQ_ENB_IRQ_TX_Msk;
                uart_op->uart_ctrl->TXFIFOIRQTRG = ui->tx_interrupt_level;
                NVIC_EnableIRQ(uart_op->tx_irq_num);
                NVIC_SetPriority(uart_op->tx_irq_num, ui->tx_irq_priority);
            }

            uart_op->uart_ctrl->ENABLE = (UART_ENABLE_RXENABLE_Msk | UART_ENABLE_TXENABLE_Msk);
            uart_op->uart_initd = true;
        } else {
            err = -1;
        }
    }
#ifdef FREE_RTOS
    serial_channel_init(ui);
#endif
    return err == 0 ? uart_op->uart_if_id : err;
}

/*
 * Tx bytes from uart without DMA
 */
int uart_write_raw(const int handle, uint8_t* tx_buf, const size_t size)
{
    int err = 0;
    for(size_t i = 0; i < size; i++){
        uint32_t timeout = 0x100000; // simple timeout
        while((uart_ops[handle].uart_ctrl->TXSTATUS & UART_TXSTATUS_WRRDY_Msk) == 0){
            timeout--;
            if(timeout == 0) { 
                err = __LINE__;
                break;
            }
        }

        if(!err){
            uart_ops[handle].uart_ctrl->DATA = tx_buf[i];
        }
    }

    return err;
}

/*
 * Get pointer to uart data address (for use as DMA source or destination address)
 */
volatile uint32_t* uart_data_ptr_get(const uart_if_id_t handle)
{
    volatile uint32_t* data_ptr = 0;
    if(handle >= 0 && handle < (int)UART_OPS_SIZE){
        data_ptr = uart_ops[handle].uart_data; 
    }
    return data_ptr; 
}

/*
 * Disable UART RX interrupt  
 */
int uart_rx_interrupt_disable(const uart_if_id_t  handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)UART_OPS_SIZE){
        NVIC_DisableIRQ(uart_ops[handle].rx_irq_num);
    } else {
        err = __LINE__;
    }

    return err;
}

/*
 * Enable UART RX interrupt  
 */
int uart_rx_interrupt_enable(const uart_if_id_t handle)
{
    int err = 0;
    if(handle >= 0 && handle < (int)UART_OPS_SIZE){
        NVIC_EnableIRQ(uart_ops[handle].rx_irq_num);
    } else {
        err = __LINE__;
    }

    return err;
}

/*
 * Set the FIFO level that the UART will interrupt at
 */
int uart_fifo_irq_trg_set(const uart_if_id_t handle, int trg)
{
    int err = 0;
    if(handle >= 0 && handle < (int)UART_OPS_SIZE){
        uart_ops[handle].uart_ctrl->RXFIFOIRQTRG = trg;
    } else {
        err = __LINE__;
    }

    return err;
}
