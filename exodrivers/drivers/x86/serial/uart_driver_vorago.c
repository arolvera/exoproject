#include "uart_driver_vorago.h"
#include <string.h>
#include <termios.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define PORT 8080

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

static uart_ops_t uart_ops[] = {{0}};




int
set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr (fd, &tty) != 0)
    {
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        return -1;
    }
    return 0;
}



#define UART_OPS_SIZE (sizeof(uart_ops) / sizeof(uart_ops[0]))



int uart_tx_status_get(int uart_handle)
{
    return 0;
}
#include <stdio.h>
int uart_init(const uart_init_t* ui)
{
    int err = 0;

    static uart_init_t u = {0};
    memcpy(&u, ui, sizeof(uart_init_t));

#if 0
    const char* pipe_name = "/opt/temp";

    u.uart_if_id = open (pipe_name, O_RDWR | O_NOCTTY | O_SYNC | O_CREAT | O_TRUNC, 0666);
    if (u.uart_if_id < 0)
    {
        printf("file descriptor failed to create\n");
        return -1;
    }

    set_interface_attribs (u.uart_if_id, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
   // set_blocking (u.uart_if_id, 1);                // set blocking
#endif


#if 0
    int opt = 1;

    int server_fd, valread;
    int new_socket = -1;
    struct sockaddr_in address;

    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET,
               SO_REUSEADDR | SO_REUSEPORT, &opt,
               sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address,
         sizeof(address));

    listen(server_fd, 3);

    while(new_socket == -1) {
        new_socket = accept(server_fd, (struct sockaddr *) &address,
                            (socklen_t *) &addrlen);
    }

    u.uart_if_id = new_socket;

    printf("serial init\n");


#endif
    serial_channel_init(&u);

    return err;
}

/*
 * Tx bytes from uart without DMA
 */
int uart_write_raw(const int handle, uint8_t* tx_buf, const size_t size)
{
    int err = 0;

    return err;
}

/*
 * Get pointer to uart data address (for use as DMA source or destination address)
 */
volatile uint32_t* uart_data_ptr_get(const uart_if_id_t handle)
{

}

/*
 * Disable UART RX interrupt  
 */
int uart_rx_interrupt_disable(const uart_if_id_t  handle)
{
    int err = 0;

    return err;
}

/*
 * Enable UART RX interrupt  
 */
int uart_rx_interrupt_enable(const uart_if_id_t handle)
{
    int err = 0;

    return err;
}

/*
 * Set the FIFO level that the UART will interrupt at
 */
int uart_fifo_irq_trg_set(const uart_if_id_t handle, int trg)
{
    int err = 0;

    return err;
}
