#include "as5047.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "stdlib.h"

SPI_HandleTypeDef * SPI_USE;


#define PI  3.1415926f
extern void Delay_Us(uint32_t nus);

static uint16_t Count_PARC(uint16_t data);
static uint16_t Command(uint16_t address,uint8_t read);
uint16_t AS_5047_Err;

static spi_read_write_buffer_fptr SPI_Read_Write;
static GPIO_TypeDef * CSN_PORT;
static uint16_t CSN_PIN;

static void spi_delay(void) {
	__NOP();
  __NOP();

}

inline void Set_CSN(GPIO_PinState x){
  HAL_GPIO_WritePin(CSN_PORT,CSN_PIN,(GPIO_PinState)x);
}  

uint8_t  Soft_SPI_Read_Write(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t length,uint32_t Timeout)
{
  uint16_t * in_buf = (uint16_t *) pRxData;
  uint16_t * out_buf = (uint16_t *) pTxData;

	for (int i = 0;i < length;i++) {
		uint16_t send = out_buf ? out_buf[i] : 0xFFFF;
		uint16_t recieve = 0;

		for (int bit = 0;bit < 16;bit++) {

      if(send>>15)
        ENCODER_MOSI_GPIO_Port->BSRR =ENCODER_MOSI_Pin;
        //HAL_GPIO_WritePin(ENCODER_MOSI_GPIO_Port,ENCODER_MOSI_Pin,GPIO_PIN_SET);
      else
        ENCODER_MOSI_GPIO_Port->BSRR =(uint32_t)ENCODER_MOSI_Pin<<16; 
        //HAL_GPIO_WritePin(ENCODER_MOSI_GPIO_Port,ENCODER_MOSI_Pin,GPIO_PIN_RESET); 
			send <<= 1;

      //HAL_GPIO_WritePin(ENCODER_CLK_GPIO_Port,ENCODER_CLK_Pin,GPIO_PIN_SET);
      ENCODER_CLK_GPIO_Port->BSRR=ENCODER_CLK_Pin;
			spi_delay();

			int samples = 0;
			samples += HAL_GPIO_ReadPin(ENCODER_MISO_GPIO_Port,ENCODER_MISO_Pin);
			__NOP();
			samples += HAL_GPIO_ReadPin(ENCODER_MISO_GPIO_Port,ENCODER_MISO_Pin);
			__NOP();
			samples += HAL_GPIO_ReadPin(ENCODER_MISO_GPIO_Port,ENCODER_MISO_Pin);
			__NOP();
			samples += HAL_GPIO_ReadPin(ENCODER_MISO_GPIO_Port,ENCODER_MISO_Pin);
			__NOP();

			recieve <<= 1;
			if (samples >= 2) {
				recieve |= 1;
			}
      ENCODER_CLK_GPIO_Port->BSRR =(uint32_t)ENCODER_CLK_Pin<<16; 
			//HAL_GPIO_WritePin(ENCODER_CLK_GPIO_Port,ENCODER_CLK_Pin,GPIO_PIN_RESET);
			spi_delay();
		}

		if (in_buf) {
			in_buf[i] = recieve;
		}
	}
  return 0;
}

uint8_t SPI_Write(uint16_t *tx,uint16_t size,uint16_t time){
  uint16_t *temp = (uint16_t *) malloc(sizeof(uint16_t)*size);
  uint8_t result = SPI_Read_Write(SPI_USE,(uint8_t *)tx,(uint8_t *)temp,size,time);
  free(temp);
  return result;
}


void As5047_Init(SPI_HandleTypeDef * spi_used,GPIO_TypeDef * csn_port,uint16_t csn_pin){
  SPI_USE = spi_used;
  CSN_PORT = csn_port;
  CSN_PIN = csn_pin;
  if(!spi_used){
    SPI_Read_Write =  Soft_SPI_Read_Write;
  }else{
    SPI_Read_Write = HAL_SPI_TransmitReceive;
  }
}

uint16_t Read_Reg(uint16_t reg){
uint16_t command=0;
uint16_t result=0;

  static uint8_t OK=0;

  //uint8_t result[2]={0};
  //uint8_t command[2]={0x7F,0xFE};
  
  command=Command(0x01,1);
  Set_CSN(0);
  OK=SPI_Write(&command,1,100);
  Set_CSN(1);
  
  command=Command(reg,1);
  Set_CSN(0);
  OK=SPI_Read_Write(SPI_USE,(uint8_t *)&command,(uint8_t *)&AS_5047_Err,1,100);
  Set_CSN(1);
  
  command=Command(0x00,1);
  Set_CSN(0);
  OK=SPI_Read_Write(SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);
  //OK=HAL_SPI_Receive(SPI_USE,(uint8_t *)&result,1,100);
  Set_CSN(1);
  if(OK!=HAL_OK){
      return 0;
  }
  return result;
}


uint16_t Get_Position(){
  uint16_t raw_data=0;
  uint16_t position=0;
  raw_data=Read_Reg(0x3FFF);
  if(raw_data&(1<<14)){
    return 0xFFFF;
  }
  position=raw_data&0x3FFF;
  return position;
}


float Position_to_Rad_Dgree(uint16_t position,uint8_t unit_type){
  float result=0;
  if(unit_type==0){
    result=2*PI*position/16384.0f;
  }else{
    result=360.0f*position/16384.0f;
  }
  return result;
}


/* 
 get the rad or dgree position
 unit_type==0  rad
 else dgree
*/
float Get_Position_Rad_Dgree(uint8_t unit_type){
  uint16_t position=0;
  float result=0;
  position=Get_Position();
  result = Position_to_Rad_Dgree(position,unit_type);
  return result;
}

static uint16_t Count_PARC(uint16_t data){
  // 偶校验
  int cnt=0;
  uint16_t temp=0;
  for(int i=0;i<15;++i){
    temp=(1<<i);
    if(data&temp){
      cnt++;
    }
  }
  if(cnt%2==0){
    return 0;
  }else{
    return temp<<1;
  }
}

static uint16_t Command(uint16_t address,uint8_t read){
  uint16_t command=0;
  command|=address;
  command|=(read<<14);
  command|=Count_PARC(command);
  return command;
}

// 本函数需确保不会被其他操作AS5047的程序打断
// 因为写入一个寄存器需要好几个SPI操作时序，一旦中间有其他操作（如读位置寄存器），则写操作失败

// 方向有两种，
// direction==0 顺时针为正 （从上往下看磁编码器）                
// direction==1 逆时针为正

void AS5047_Set_Direction(uint8_t direction){
  uint16_t command=0;
  static uint16_t result=0;

  command=Command(0x01,1);
  Set_CSN(0);
  SPI_Write(&command,1,100);
  Set_CSN(1);

  command=Command(0x18,0);
  Set_CSN(0);
  SPI_Read_Write(SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   // 接收错误寄存器
  Set_CSN(1);

  direction<<=2;
  command=Command(direction,0);
  Set_CSN(0);
  SPI_Read_Write(SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   // 接收寄存器原值
  Set_CSN(1);

  command=Command(0x00,1);
  Set_CSN(0);
  SPI_Read_Write(SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   /// 发送NOP，接收寄存器新值
  Set_CSN(1);
}
