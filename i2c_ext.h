#ifndef __I2C_EX_H
#define __I2C_EX_H


#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "main.h"


void I2C_EXT_Init(I2C_HandleTypeDef * i2c,GPIO_TypeDef * scl_gpio_port,uint32_t scl_gpio_pin,\
GPIO_TypeDef * sda_gpio_port,uint32_t sda_gpio_pin,void * i2c_init_func);

#endif