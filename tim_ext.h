#ifndef __TIM_EXT_H
#define __TIM_EXT_H
#include "stdlib.h"
#include "tim.h"


typedef struct{
    uint32_t start_cnt;
    uint32_t start_ms;
}Time_Counter;


void Timer_1ms_IRQ_Handler();
void Timer_Init(TIM_HandleTypeDef *tim);
void Time_Counter_Start(Time_Counter * time_counter);
uint32_t Time_Counter_Stop(Time_Counter * time_counter);
void Timer_1ms_IRQ_Handler();
#endif