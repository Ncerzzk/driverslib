
#ifndef __OLED_H
#define __OLED_H			  	 
//#include "iostm8s103k3.h"
//#include"iostm8s208MB.h"
#include "stm32f1xx_hal.h"
#include "main.h"

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//****************复位*********************

#define OLED_RST_L  HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin, GPIO_PIN_RESET)
#define OLED_RST_H  HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin, GPIO_PIN_SET)

//****************数据/命令***************

#define OLED_DC_L  HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_H  HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin, GPIO_PIN_SET)

//****************片选*********************

#define OLED_CS_L  HAL_GPIO_WritePin(OLED_CSN_GPIO_Port,OLED_CSN_Pin, GPIO_PIN_RESET)
#define OLED_CS_H  HAL_GPIO_WritePin(OLED_CSN_GPIO_Port,OLED_CSN_Pin, GPIO_PIN_SET)








//OLED模式设置
//0:4线串行模式
//1:并行8080模式

#define SIZE 16
#define XLevelL		0x02
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  
//-----------------OLED端口定义----------------  					   
#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long


void delay_ms(uint16_t ms);

//OLED控制用函数
void  OLED_WR_Byte(uint8_t dat,uint8_t cmd);
void  OLED_Display_On(void);
void  OLED_Display_Off(void);
void  OLED_Init(void);
void  OLED_Clear(void);
void  OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void  OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void  OLED_Set_Pos(unsigned char x, unsigned char y);
void  OLED_GPIO_Init(void);


#endif  




