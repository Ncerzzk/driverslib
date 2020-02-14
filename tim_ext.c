#include "tim_ext.h"



typedef struct{
    uint32_t start_cnt;
    uint32_t start_ms;
}Time_Counter;

static TIM_HandleTypeDef *Timer_Tim;
uint32_t TIM_MAX_FREQ;
static uint32_t Pass_Time_ms;

void Time_Counter_Start(Time_Counter * time_counter){
    time_counter->start_cnt=Timer_Tim->Instance->CNT;
    time_counter->start_ms=Pass_Time_ms;
}

uint32_t Time_Counter_Stop(Time_Counter * time_counter){
    uint32_t cnt_sub=Timer_Tim->Instance->CNT-time_counter->start_cnt;
    uint32_t ms_sub=Pass_Time_ms-time_counter->start_ms;
    uint32_t result;

    result=ms_sub*1000+cnt_sub;
    return result;
}

void Timer_Init(TIM_HandleTypeDef *tim,uint32_t max_freq)
{
    Timer_Tim = tim;
    TIM_MAX_FREQ=max_freq;

    tim->Instance->PSC=83;
    tim->Instance->ARR=TIM_MAX_FREQ/(tim->Instance->PSC+1)/1000-1;

    HAL_TIM_Base_Start_IT(tim);
}

void Timer_1ms_IRQ_Handler(){
    Pass_Time_ms++;
}

