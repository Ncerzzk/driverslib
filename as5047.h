#ifndef __AS5047_H
#define __AS5047_H
#include "stm32f4xx_hal.h"
uint16_t Read_Reg(uint16_t reg);
uint16_t Get_Position();
float Get_Position_Rad_Dgree(uint8_t unit_type);
void AS5047_Set_Direction(uint8_t direction);
#endif