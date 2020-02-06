#include "usart.h"
#include "uart_ext.h"
#include "command.h"
#include "cmd_fun.h"


#define SPECIAL_LARGE_RECEIVE

static UART_TXRX_Mode Tx_Mode=BLOCK;
static UART_TXRX_Mode Rx_Mode=IT;
static UART_HandleTypeDef * debug_uart=0;

uint8_t uart_buffer[100+1];

uint8_t buffer_rx_temp;
#ifdef SPECIAL_LARGE_RECEIVE
  #define BUFFER_RX_SIZE  1001
#else
  #define BUFFER_RX_SIZE 30
#endif 
uint8_t buffer_rx[BUFFER_RX_SIZE];
int buffer_rx_count=0;
uint8_t buffer_rx_OK;

void UART_Start_Receive(){
    if(Rx_Mode==IT){
        HAL_UART_Receive_IT(debug_uart,&buffer_rx_temp,1);
    }else if(Rx_Mode==DMA){
      __HAL_UART_ENABLE_IT(debug_uart,UART_IT_IDLE);
      HAL_UART_Receive_DMA(debug_uart, (uint8_t *)buffer_rx, BUFFER_RX_SIZE-1);
    }
}



void debug_uart_init(UART_HandleTypeDef *uart,UART_TXRX_Mode tx_mode,UART_TXRX_Mode rx_mode){
  // 本文件使用方式
  // 调用本函数
  // 如果接收使用DMA 方式
  // 则需要在stm32_XX_it.c中，在对应的串口中断服务函数中，调用HAL_UART_IDLECallback
  // 因为HAL库到现在在串口中断处理函数中都没增加对空闲中断的处理，只能出此下策。
  
    Tx_Mode=tx_mode;
    Rx_Mode=rx_mode;
    debug_uart=uart;

    command_init();
    UART_Start_Receive();
}


static void UART_Send(uint8_t * data,uint16_t size){
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

void uprintf_polling(char *fmt,...){
  int size;
  
  va_list arg_ptr;
  
  va_start(arg_ptr, fmt);  
  
  size=vsnprintf((char*)uart_buffer, 100 + 1, fmt, arg_ptr);
  va_end(arg_ptr);
  HAL_UART_Transmit(debug_uart,uart_buffer,size,1000);
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

#ifndef SPECIAL_LARGE_RECEIVE
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
#endif

void HAL_UART_IDLECallback(UART_HandleTypeDef *huart){
   uint8_t temp=0;
  
  __HAL_UART_CLEAR_IDLEFLAG(huart);
  if(huart==debug_uart){
    HAL_UART_DMAStop(debug_uart);
    temp= huart->Instance->SR;
    temp= huart->Instance->DR;//读出串口的数据，防止在关闭DMA期间有数据进来，造成ORE错误
    temp++;  // 无用，单纯为了消掉warning
    #ifdef SPECIAL_LARGE_RECEIVE
      UART_Large_Reveice();
      UART_Start_Receive();
    #else
      buffer_rx_OK=1;
    #endif
  }
  
}

void UART_Command_Analize_And_Call(){
  analize(buffer_rx);
  UART_Start_Receive();
  buffer_rx_OK=0;
}


__weak void UART_Large_Reveice(){

}