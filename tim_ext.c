#include "tim_ext.h"
#include "stdlib.h"



static TIM_HandleTypeDef *Timer_Tim;
uint32_t TIM_MAX_FREQ;
static uint32_t Pass_Time_ms;
uint32_t US_CNT;

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
    US_CNT=TIM_MAX_FREQ/1000/1000;

    tim->Instance->PSC=max_freq/1000/1000;
    tim->Instance->ARR=TIM_MAX_FREQ/(tim->Instance->PSC+1)/1000-1;

    HAL_TIM_Base_Start_IT(tim);
}

void Timer_1ms_IRQ_Handler(){
    Pass_Time_ms++;
}

static TIM_HandleTypeDef *Delay_Timer_Tim;
void Delay_Timer_Init(TIM_HandleTypeDef *tim,uint32_t max_freq){
    Delay_Timer_Tim = tim;
    tim->Instance->PSC = max_freq/1000/1000;
    tim->Instance->ARR = 0xFFFF;
}

// This implement is not thread safe, so I add the Delaying flag to prevent deadlock
// there are some ways to solve this problem
// but i'm lazy to use C program language to do so, I don't want to implement more datastructures by C
// may be next time  when I'm using rust, the problem could be solved

// 线程仍是不安全
void Delay_Us(uint16_t n){
    static uint8_t Delaying=0;
    while(Delaying);
    
    Delay_Timer_Tim->Instance->CNT=0; 
    HAL_TIM_Base_Start(Delay_Timer_Tim);
    Delaying = 1;
    while(Delay_Timer_Tim->Instance->CNT<n){
        ;
    }
    HAL_TIM_Base_Stop(Delay_Timer_Tim);
    Delaying = 0;
}

void Delay_Us_NOP_Init(uint32_t max_freq){
   US_CNT=max_freq/1000/1000;
}

void Delay_Us_NOP(uint16_t n){
    for(int i=0;i<n;++i){
        for(int j=0;j<US_CNT;++j){
            __NOP();
        }
    }
}