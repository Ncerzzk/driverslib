#include "music_and_voice.h"
#include "foc.h"

extern Uvect_Mos U0,U4,U6;
struct
{
    uint32_t ms_cnt;
    uint32_t cnt;
    uint32_t TIM_old_ARR;
    float time;   // 一个音符持续的时间 range from 0-1
    uint8_t stop;

    TIM_HandleTypeDef * USE_TIM;
    uint32_t TIM_MAX_FREQ;
    uint16_t BEAT_cnt;  // 节拍计数值
    float DUTY;
} Music_Info;


typedef enum
{
    STOP_NOTE = 20000,
    DO = 261,
    RE = 293,
    MI = 329,
    FA = 349,
    SOL = 392,
    LA = 440,
    SI = 493
} MUSIC_FREQ;

MUSIC_FREQ Music_Freqs[8] = {STOP_NOTE, DO, RE, MI, FA, SOL, LA, SI};
//uint16_t Music_Song[]={0x1803,0x1802,0x1801,0x806,0x1801,0x1802,0x1803,0x1805,0x1803,0x1802,0x1801,0x806,0x806,0x805,0x403,0x800,0x1f01,0xf07,0x806,0x805,0x806,0x1801,0x1801,0x1805,0x1803,0x1f03,0x1f02,0x1801,0x1801,0x1801,0x806,0x407,0x800,0x1f03,0x1f02,0x1803,0x1802,0x1803,0x1805,0x1806,0x1805,0x1f05,0x1805,0x1f03,0x1f06,0x1805,0x1f04,0x1f05,0x1f04,0x1f05,0x1f04,0x1403,0x800,0x1f02,0x1f03,0x1804,0x1803,0x1802,0xf00,0xf06,0x1803,0x1f02,0x1f01,0x1f01,0x1f01,0x1f01,0x1f02,0xf07,0x807,0xf07,0xf07,0x807,0xf06,0xf06,0x807,0x407};    // 音符数组   3,3,3,3,2,3
uint16_t Music_Song[] = {0x803, 0x803, 0x803, 0x802, 0x203, 0x802, 0x803, 0x802, 0x802, 0x3406, 0x3806, 0x3807, 0x401, 0x802, 0x801, 0x3407, 0x3805, 0x3106, 0x803, 0x803, 0x803, 0x802, 0x403, 0x806, 0x805, 0x806, 0x805, 0x805, 0x402, 0x802, 0x803, 0x804, 0x805, 0x804, 0x403, 0x802, 0x103, 0x406, 0x3807, 0x3806, 0x405, 0x803, 0x105, 0x803, 0x805, 0x402, 0x806, 0x805, 0x403, 0x802, 0x103, 0x402, 0x805, 0x406, 0x401, 0x806, 0x806, 0x406, 0x806, 0x807, 0x1401, 0x807, 0x806, 0x407, 0x805, 0x103, 0x406, 0x807, 0x806, 0x405, 0x803, 0x105, 0x804, 0x805, 0x406, 0x807, 0x806, 0x407, 0x405, 0xf06, 0x103, 0x402, 0x805, 0x406, 0x401, 0x806, 0x406, 0x806, 0x807, 0x1401, 0x807, 0x806, 0x407, 0x805, 0x106};
/*
3/,3/,3/,2/,3-,2/,3/,2/,2/,-6,-6/,-7/,1,2/,1/,-7
*/

void Music_Init(TIM_HandleTypeDef * tim,uint8_t beat_cnt_per_minute,float duty){
    Music_Info.USE_TIM=tim;
    if(tim->Instance!=TIM1 && tim->Instance!=TIM8){
        Music_Info.TIM_MAX_FREQ=84000000;
    }else{
        Music_Info.TIM_MAX_FREQ=168000000;
    }
    Music_Info.BEAT_cnt=1000*60/beat_cnt_per_minute;

    Music_Info.DUTY=duty;

}

#define HALF_NOTE_K 1.059f
float Music_Get_Note(uint16_t num, float *time)
{
    uint8_t index = num & 0xFF;
    MUSIC_FREQ note = Music_Freqs[index];

    float result = note;

    uint8_t t = (num & 0xF00) >> 8;
    uint8_t octave = (num & 0x3000) >> 12;
    uint8_t semitone = (num & 0xC000) >> 14;
    switch (t)
    {
    case 1:
        *time = 4.0f;
        break;
    case 2:
        *time = 2.0f;
        break;
    case 4:
        *time = 1.0f;
        break;
    case 8:
        *time = 0.5f;
        break;
    case 0xF:
        *time = 0.25f;
        break;

    default:
        break;
    }

    switch (octave)
    {
    case 1:
        result = note * 2;
        break;
    case 0:
        result = note;
        break;
    case 3:
        result = note / 2;
        break;
    }

    switch (semitone)
    {
    case 1:
        result *= HALF_NOTE_K;
        break;
    case 0:
        break;
    case 3:
        result /= HALF_NOTE_K;
        break;
    }

    return result;
}

void Music_Play_Note(uint16_t freq)
{
    freq*=2;
    Music_Info.USE_TIM->Instance->PSC = 19;
    Music_Info.USE_TIM->Instance->ARR = Music_Info.TIM_MAX_FREQ/20/ freq - 1;
}


void Muisc_Play_TIM_IRQ_Handler()
{
    static uint8_t first_time = 0;
    if (Music_Info.stop == 1)
    {
        Set_Vector(U0, 0);
        return;
    }

    if (!first_time)
    {
        Set_Vector(U4, Music_Info.DUTY);
    }
    else
    {
        Set_Vector(U6, Music_Info.DUTY);
    }
    first_time = !first_time;
}


void Music_Play_Beat()
{ // 节拍处理，暂定一分钟80拍，即750ms一拍
    static uint16_t next_time = 750;
    uint16_t note;
    /*
    if (Board_Mode != SING_MODE)
    {
        return;
    }
    */
    Music_Info.ms_cnt++;

    if (Music_Info.ms_cnt >= next_time * 0.85 && Music_Info.ms_cnt < next_time)
    {
        Music_Info.stop = 1;
    }

    if (Music_Info.ms_cnt >= next_time)
    {                          // 750*1/4
        Music_Info.ms_cnt = 0; // 以下代码1s执行一次
        Music_Info.stop = 0;
        Music_Info.time = 1.0f;

        uint16_t temp = Music_Song[Music_Info.cnt];
        note = (uint16_t)Music_Get_Note(temp, &Music_Info.time);

        Music_Play_Note(note);
        next_time = Music_Info.BEAT_cnt * Music_Info.time;
        if (Music_Info.cnt == sizeof(Music_Song) / sizeof(uint16_t) - 1)
        {
            Music_Info.cnt=0;
        }
        else
        {
            Music_Info.cnt++;
        }
    }
}

struct
{
    uint32_t cnt;
    TIM_HandleTypeDef * USE_TIM;
    uint32_t TIM_MAX_FREQ;
    uint16_t Fs;
    float voice_duty;
} Voice_Info;

uint8_t Voice_Data[6000];
uint16_t Voice_Data_Head_Index;
uint16_t Voice_Data_End_Index;
uint8_t Voice_Init_OK;
/*
void Voice_Init(TIM_HandleTypeDef * tim,uint16_t fs){

    Voice_Info.USE_TIM=tim;
    if(tim->Instance!=TIM1 && tim->Instance!=TIM8){
        Voice_Info.TIM_MAX_FREQ=84000000;
    }else{
        Voice_Info.TIM_MAX_FREQ=168000000;
    }
    Voice_Info.Fs=fs;

    tim->Instance->PSC=0;
    tim->Instance->ARR=Voice_Info.TIM_MAX_FREQ/fs;
}
*/
#include "string.h"
#include "uart_ext.h"
void request_for_bytes(){
    uprintf("r");
}

void Voice_Fshz_Handler()
{
    if(!Voice_Init_OK){
        return ;
    }
    if(Voice_Data_Head_Index%1000==0){
        request_for_bytes();
    }
    Voice_Data_Head_Index++;
    if (Voice_Data_Head_Index >= sizeof(Voice_Data))
    {
        Voice_Data_Head_Index=0;
    }

    Voice_Info.voice_duty = (float)Voice_Data[Voice_Data_Head_Index] / 255.0f;

    if(Voice_Info.voice_duty>0.5f){
       Set_Vector(U4, Voice_Info.voice_duty-0.5f); 
    }else{
       Set_Vector(U6, Voice_Info.voice_duty-0.5f);  
    }
}

extern uint8_t buffer_rx[];
void UART_Large_Reveice(){
    memcpy(Voice_Data+Voice_Data_End_Index,buffer_rx,1000);
    Voice_Data_End_Index+=1000;
    if(Voice_Data_End_Index>=sizeof(Voice_Data)){
        Voice_Data_End_Index%=sizeof(Voice_Data);
    }
}



void Voice_Init(){
    // 先要他个5000个字节先
    for(int i=0;i<5;++i){
        request_for_bytes();
        HAL_Delay(2000);
    }
    Voice_Init_OK=1;
}
#include "usart.h"
#include "uart_ext.h"
extern uint8_t buffer_rx_temp;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    Voice_Info.voice_duty = (float)buffer_rx_temp / 255.0f;

    if(Voice_Info.voice_duty>0.5f){
       Set_Vector(U4, Voice_Info.voice_duty-0.5f); 
    }else{
       Set_Vector(U6, Voice_Info.voice_duty-0.5f);  
    }
    HAL_UART_Receive_IT(huart,&buffer_rx_temp,1);
}