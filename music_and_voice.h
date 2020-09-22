#ifndef __MUSICE_AND_VOICE_H
#define __MUSICE_AND_VOICE_H

#include "tim.h"

void Music_Play_Beat();
void Music_Init(TIM_HandleTypeDef * tim,uint8_t beat_cnt_per_minute,float duty);
void Voice_Init();
void UART_Large_Reveice();
#endif