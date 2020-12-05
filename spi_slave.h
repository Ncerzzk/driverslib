#ifndef __SPI_SLAVE_H
#define __SPI_SLAVE_H

#include "stdint.h"
#include "spi.h"

typedef struct{
    uint8_t reg_addr;
    uint32_t * val_ptr;
    uint8_t val_size;
}SPI_COM_VAL;

void SPI_Slave_Init(SPI_HandleTypeDef * hspi,uint16_t reg_list_len);
void SPI_Slave_CSN_Handler(uint8_t flag);
void SPI_COM_Master_Init(SPI_HandleTypeDef * master_hspi,GPIO_TypeDef* csn_port, uint16_t csn_pin,void (*delay_us_func) (int));
void SPI_COM_Master_Read_Reg(uint8_t reg_addr,uint8_t * reg_data,uint16_t size);
void SPI_COM_Master_Write_Reg(uint8_t reg_addr,uint8_t * reg_data,uint16_t size);
#endif