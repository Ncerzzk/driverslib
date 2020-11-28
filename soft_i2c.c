#include "soft_i2c.h"

#include "stdlib.h"
#include "string.h"

static GPIO_TypeDef *SCL_Port;
static GPIO_TypeDef *SDA_Port;

static uint16_t SDA_Pin;
static uint16_t SCL_Pin; 

#define SDA_H()  HAL_GPIO_WritePin(SDA_Port,SDA_Pin,GPIO_PIN_SET)
#define SDA_L()  HAL_GPIO_WritePin(SDA_Port,SDA_Pin,GPIO_PIN_RESET)

#define SCL_H()  HAL_GPIO_WritePin(SCL_Port,SCL_Pin,GPIO_PIN_SET)
#define SCL_L()  HAL_GPIO_WritePin(SCL_Port,SCL_Pin,GPIO_PIN_RESET)

#define SDA_Read() HAL_GPIO_ReadPin(SDA_Port,SDA_Pin)
#define SCL_Read() HAL_GPIO_ReadPin(SCL_Port,SCL_Pin)

void Soft_I2C_Init(GPIO_TypeDef * scl_port,uint16_t scl_pin,GPIO_TypeDef * sda_port,uint16_t sda_pin){

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    SCL_Port = scl_port;
    SDA_Port = sda_port;
    SDA_Pin = sda_pin;
    SCL_Pin = scl_pin;

    HAL_GPIO_WritePin(SDA_Port,SDA_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(SCL_Port,SCL_Pin,GPIO_PIN_SET);
    GPIO_InitStruct.Pin = SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SDA_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SCL_Pin;
    HAL_GPIO_Init(SCL_Port, &GPIO_InitStruct); 

}

void I2C_delay(void)
{
  for(int i=0;i<20;++i){
    	__NOP();
  }
}


int I2C_Start(void){
	SDA_H();
	SCL_H();
	I2C_delay();
	if(!SDA_Read())
	{
		return DISABLE;
	}
    SDA_L();
	I2C_delay();
	if(SDA_Read())
	{
		return DISABLE;
	}
    SCL_L();
	return ENABLE;
}


uint8_t I2C_GetAck(void)
{
  uint8_t time = 0;
	SDA_H();
	I2C_delay();
	SCL_H();
	I2C_delay();
	while(SDA_Read())
	{
		time++;
		if(time > 250)
		{			
			SCL_L();
			return DISABLE;
		}
	}
	SCL_L();
	return ENABLE;
}

static void I2C_Ack()
{
	SCL_L();
	I2C_delay();
	SDA_L();
	I2C_delay();
	SCL_H();
	I2C_delay();
	SCL_L();
  I2C_delay();
  SDA_H();
  
}
 
static void I2C_NoAck()
{
	SCL_L();	
	I2C_delay();
	SDA_H();
	I2C_delay();
	SCL_H();
	I2C_delay();
	SCL_L();
}

void I2C_SendByte(uint8_t Data){
  uint8_t cnt;	
  for(cnt=0; cnt<8; cnt++)
  {
    SCL_L();                              
    I2C_delay();
 
    if(Data & 0x80)
    {
      SDA_H();                         
    }
    else
    {
      SDA_L();                         
    }
    Data <<= 1;
    SCL_H();                              
    I2C_delay();
  }
  SCL_L();                                   
  I2C_delay();
}

uint8_t I2C_ReadByte(uint8_t ack)
{
  uint8_t cnt;
  uint16_t data=0;
	
  for(cnt=0; cnt<8; cnt++){
    SCL_L();                                
    I2C_delay();
		
    SCL_H();                             
    data <<= 1;
    if(SDA_Read())
    {
      data |= 0x01;                              
    }
    I2C_delay();
  }
  if(ack == 1){
     I2C_Ack();
  }
  else
  {
     I2C_NoAck();
  }
  return data;                                  
}

void I2C_Stop(void)
{
	SCL_L();
	SDA_L();	
	SCL_H();
	I2C_delay();
	SDA_H();
	I2C_delay();
}

uint8_t I2C_Write_Buffer(uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len){
    uint8_t * data;
    uint8_t result;
    data=(uint8_t *)malloc((len+1)*sizeof(uint8_t));
    memcpy(data+1,pBuffer,len);
    data[0]=writeAddr;

    I2C_Start();
    I2C_SendByte(slaveAddr);
    I2C_GetAck();
    for(int i=0;i<len+1;++i){
        I2C_SendByte(data[i]);
        I2C_GetAck();
    }
    I2C_Stop();
    free(data);
    return 0;
}


uint8_t I2C_Read_Buffer(uint8_t slaveAddr,uint8_t readAddr,uint8_t *pBuffer,uint16_t len){
    I2C_Start();
    I2C_SendByte(slaveAddr);
    I2C_GetAck();
    I2C_SendByte(readAddr);
    I2C_GetAck();


    I2C_Start();
    I2C_SendByte(slaveAddr|1);
    I2C_GetAck();

    for(int i=0;i<len;++i){
        if(i!=len-1)
            pBuffer[i] = I2C_ReadByte(1);
        else{
           pBuffer[i] = I2C_ReadByte(0); 
        }
    }
    I2C_Stop();
    return 0;
}

void I2C_Reset(void){

}