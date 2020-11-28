#include "debug_utils.h"
#include "uart_ext.h"

uint8_t Wave_ID=99;
uint8_t Wave_Flag=0;

void send_debug_wave(Wave_Group * wg){
    if(Wave_Flag&&Wave_ID==wg->wave_id)
        send_wave(*wg->ch1,*wg->ch2,*wg->ch3,*wg->ch4);
}