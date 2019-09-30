#include "usart.h"
#include "uart_ext.h"



static UART_TXRX_Mode Tx_Mode=BLOCK;
static UART_TXRX_Mode Rx_Mode=IT;
static UART_HandleTypeDef * debug_uart=0;


uint8_t buffer_rx_temp;

uint8_t buffer_rx[30];
int buffer_rx_count=0;
uint8_t buffer_rx_OK;


void debug_uart_init(UART_HandleTypeDef *uart,UART_TXRX_Mode tx_mode,UART_TXRX_Mode rx_mode){
    Tx_Mode=tx_mode;
    Rx_Mode=rx_mode;
    debug_uart=uart;

    if(Rx_Mode==IT){
        HAL_UART_Receive_IT(debug_uart,&buffer_rx_temp,1);
    }else if(Rx_Mode==DMA){
        // 开空闲中断
    }
}

inline void UART_Send(uint8_t * data,uint16_t size){
    switch (Tx_Mode)
    {
    case BLOCK:
        HAL_UART_Transmit(debug_uart,data,size,1000);
        break;
    case IT:
        HAL_UART_Transmit_IT(debug_uart,data,size);
        break;
    case DMA:
        HAL_UART_Transmit_DMA(debug_uart,data,size);
        break;
    default:
        break;
    }
}

char s[22]={'b','y',16,6};
void send_wave(float arg1,float arg2,float arg3,float arg4){

  s[20]='\r';
  s[21]='\n';
  memcpy(s+4,&arg1,sizeof(arg1));
  memcpy(s+8,&arg2,sizeof(arg1));
  memcpy(s+12,&arg3,sizeof(arg1));
  memcpy(s+16,&arg4,sizeof(arg1));
  UART_Send((uint8_t *)s,sizeof(s));
}

void uprintf(char *fmt, ...)
{
  int size;
  
  va_list arg_ptr;
  
  va_start(arg_ptr, fmt);  
  
  size=vsnprintf((char*)uart_buffer, 100 + 1, fmt, arg_ptr);
  va_end(arg_ptr);
  UART_Send(uart_buffer,size);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart==debug_uart){
    buffer_rx[buffer_rx_count]=buffer_rx_temp;
    if(buffer_rx[buffer_rx_count-1]=='\r'&&buffer_rx[buffer_rx_count]=='\n'){
      buffer_rx[buffer_rx_count-1]='\0';
      buffer_rx_OK=1;
      // call
      buffer_rx_count=0;
    }else{
      buffer_rx_count++;
      HAL_UART_Receive_IT(huart,&buffer_rx_temp,1);
    }
  }
}
