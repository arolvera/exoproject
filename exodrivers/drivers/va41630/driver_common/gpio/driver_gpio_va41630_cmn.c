#include "gpio/hal_gpio.h"
#include "device.h"

inline void gpio_init(hal_gpio_t *gpio, GPIO_DIRECTION dir)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |=
          (CLK_ENABLE_PORTA | CLK_ENABLE_PORTB | CLK_ENABLE_PORTC |
          CLK_ENABLE_PORTD | CLK_ENABLE_PORTE | CLK_ENABLE_PORTF |
          CLK_ENABLE_PORTG | CLK_ENABLE_IOCONFIG);

    if(dir == GPIO_INPUT){
        VOR_GPIO_IN(((VOR_GPIO_Type *)gpio->port), gpio->pin);
    }else{
        VOR_GPIO_OUT(((VOR_GPIO_Type *)gpio->port), gpio->pin);
    }

}

inline void gpio_set(hal_gpio_t *gpio, uint8_t val)
{
    if(val){
        VOR_GPIO_SET(((VOR_GPIO_Type *)gpio->port), gpio->pin);
    }else{
        VOR_GPIO_CLR(((VOR_GPIO_Type *)gpio->port), gpio->pin);
    }

}

inline uint8_t gpio_rd(hal_gpio_t *gpio)
{
    return VOR_GPIO_RD(((VOR_GPIO_Type *)gpio->port), gpio->pin);
}
