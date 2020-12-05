#ifndef __SPI_SLAVE_H
#define __SPI_SLAVE_H

#include "stdint.h"
#include "spi.h"
#include "gpio.h"

typedef struct{
    uint8_t reg_addr;
    uint32_t * val_ptr;
    uint8_t val_size;
}SPI_COM_VAL;

typedef struct{
   SPI_HandleTypeDef * SPI_Use;
   void (*Delay_Us_Func)(uint16_t);
   GPIO_TypeDef * CSN_Port;
   uint16_t CSN_Pin;
   uint8_t Rx_Buffer[10]; 
}SPI_Com_Master;


void SPI_Slave_Init(SPI_HandleTypeDef * hspi,uint16_t reg_list_len);
void SPI_Slave_CSN_Handler(uint8_t flag);
void SPI_COM_Master_Init(SPI_Com_Master *,SPI_HandleTypeDef * master_hspi,GPIO_TypeDef* csn_port, uint16_t csn_pin,void (*delay_us_func) (uint16_t));
void SPI_COM_Master_Read_Reg(SPI_Com_Master *,uint8_t reg_addr,uint8_t * reg_data,uint16_t size);
void SPI_COM_Master_Write_Reg(SPI_Com_Master *,uint8_t reg_addr,uint8_t * reg_data,uint16_t size);
#endif