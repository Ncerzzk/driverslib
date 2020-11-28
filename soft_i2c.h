#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H
#include "stdint.h"
#include "gpio.h"

void Soft_I2C_Init(GPIO_TypeDef * scl_port,uint16_t scl_pin,GPIO_TypeDef * sda_port,uint16_t sda_pin);
uint8_t I2C_Write_Buffer(uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len);
uint8_t I2C_Read_Buffer(uint8_t slaveAddr,uint8_t readAddr,uint8_t *pBuffer,uint16_t len);
void I2C_Reset(void);
#endif