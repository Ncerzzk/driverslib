#ifndef __AS5047_H
#define __AS5047_H
#include "stm32f4xx_hal.h"
uint16_t Read_Reg(uint16_t reg);
uint16_t Get_Position();
float Get_Position_Rad_Dgree(uint8_t unit_type);
void AS5047_Set_Direction(uint8_t direction);

typedef uint8_t (*spi_read_write_buffer_fptr)(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,
                                          uint32_t Timeout);

void As5047_Init(SPI_HandleTypeDef * spi_used,GPIO_TypeDef * csn_port,uint16_t csn_pin);
float Position_to_Rad_Dgree(uint16_t position,uint8_t unit_type);

#endif