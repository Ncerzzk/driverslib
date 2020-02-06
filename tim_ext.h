#ifndef __TIM_EXT_H
#define __TIM_EXT_H
#include "stdlib.h"
#include "tim.h"

void Timer_1ms_IRQ_Handler();
void Timer_Init(TIM_HandleTypeDef *tim);
#endif