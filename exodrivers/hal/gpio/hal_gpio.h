//
// Created by marvin on 5/23/23.
//

#ifndef MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_GPIO_HAL_GPIO_H_
#define MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_GPIO_HAL_GPIO_H_
#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint32_t *port;    //Pointer to gpio address base
    uint16_t pin;      //gpio pin
}hal_gpio_t;

typedef enum{
    GPIO_INPUT = 0,
    GPIO_OUTPUT
}GPIO_DIRECTION;

void gpio_init(hal_gpio_t *gpio,  GPIO_DIRECTION dir);
void gpio_set(hal_gpio_t *gpio, uint8_t val);
uint8_t gpio_rd(hal_gpio_t *gpio);

#endif//MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXODRIVERS_HAL_GPIO_HAL_GPIO_H_
