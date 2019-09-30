#include "i2c.h"
#include "gpio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#define OUTTIME 10
I2C_HandleTypeDef * I2C_USE;
GPIO_TypeDef * SCL_GPIO_PORT;
GPIO_TypeDef * SDA_GPIO_PORT;
uint32_t SCL_GPIO_PIN;
uint32_t SDA_GPIO_PIN;

void * I2C_Init_Func;

void I2C_EXT_Init(I2C_HandleTypeDef * i2c,GPIO_TypeDef * scl_gpio_port,uint32_t scl_gpio_pin,\
GPIO_TypeDef * sda_gpio_port,uint32_t sda_gpio_pin,void * i2c_init_func){
    I2C_USE=i2c;
    SCL_GPIO_PORT=scl_gpio_port;
    SCL_GPIO_PIN=scl_gpio_pin;

    SDA_GPIO_PORT=sda_gpio_port;
    SDA_GPIO_PIN=sda_gpio_pin;

    I2C_Init_Func=i2c_init_func;
}


uint8_t I2C_Read_Buffer(uint8_t slaveAddr,uint8_t readAddr,uint8_t *pBuffer,uint16_t len){
  uint8_t result;
  result=HAL_I2C_Master_Transmit(I2C_USE,slaveAddr,&readAddr,1,OUTTIME);
  if(result!=HAL_OK){
    return result;
  }
  return HAL_I2C_Master_Receive(I2C_USE,slaveAddr,pBuffer,len,OUTTIME);		 
}


uint8_t I2C_Write_Buffer(uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len){
  uint8_t * data;
  uint8_t result;
  data=(uint8_t *)malloc((len+1)*sizeof(uint8_t));
  memcpy(data+1,pBuffer,len);
  data[0]=writeAddr;
  result=HAL_I2C_Master_Transmit(I2C_USE,slaveAddr,data,len+1,OUTTIME);
  free(data);
  return result;
}

inline void Delay_Us(uint32_t nus){
    // 主频按72M 计算，其他主频略有误差
    uint32_t cnt=nus*72;

    while(cnt--);
}
void I2C_Reset(){
    

    GPIO_InitTypeDef GPIO_InitStruct;


    GPIO_InitStruct.Pin = SCL_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SCL_GPIO_PORT, &GPIO_InitStruct); 

    GPIO_InitStruct.Pin = SDA_GPIO_PIN;
    HAL_GPIO_Init(SDA_GPIO_PORT, &GPIO_InitStruct); 
    
    for(int i=0;i<9;++i){
      if(HAL_GPIO_ReadPin(SDA_GPIO_PORT,SDA_GPIO_PIN)!=GPIO_PIN_SET){
        HAL_GPIO_WritePin(SCL_GPIO_PORT,SCL_GPIO_PIN,GPIO_PIN_SET);
        Delay_Us(10);
        HAL_GPIO_WritePin(SCL_GPIO_PORT,SCL_GPIO_PIN,GPIO_PIN_RESET);
        Delay_Us(10);
      }else{
        break;
      }
    }

    HAL_GPIO_WritePin(SDA_GPIO_PORT,SDA_GPIO_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(SCL_GPIO_PORT,SCL_GPIO_PIN,GPIO_PIN_SET);

    I2C_USE->State == HAL_I2C_STATE_RESET;
    (*(void (*)(void))  I2C_Init_Func)();
}