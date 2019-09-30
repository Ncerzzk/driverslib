#ifndef __UART_EX_H
#define __UART_EX_H
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum{
    BLOCK=0x00,
    IT=0x01,
    DMA=0x02
}UART_TXRX_Mode;


uint8_t uart_buffer[100 + 1];


void send_wave(float arg1,float arg2,float arg3,float arg4);
void uprintf(char *fmt, ...);
void debug_uart_init(UART_HandleTypeDef *uart,UART_TXRX_Mode tx_mode,UART_TXRX_Mode rx_mode);

#endif