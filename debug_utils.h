#ifndef __DEBUG_UTILS_H
#define __DEBUG_UTILS_H
#include "stdint.h"

typedef struct _wave_group
{
    uint8_t wave_id;
    float * ch1,*ch2,*ch3,*ch4;
}Wave_Group;

void send_debug_wave(Wave_Group * wg);
extern uint8_t Wave_Flag;
extern uint8_t Wave_ID;
#endif