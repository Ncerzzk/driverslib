#include "as5047.h"
#include "spi.h"
#include "usart.h"

#define SPI_USE hspi3
#define Set_CSN(x)      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,(GPIO_PinState)x);
#define PI  3.1415926f
extern void Delay_Us(uint32_t nus);

static uint16_t Count_PARC(uint16_t data);
static uint16_t Command(uint16_t address,uint8_t read);
uint16_t AS_5047_Err;

uint16_t Read_Reg(uint16_t reg){
  uint16_t command=0;
  uint16_t result=0;

  static uint8_t OK=0;

  //uint8_t result[2]={0};
  //uint8_t command[2]={0x7F,0xFE};
  
  command=Command(0x01,1);
  Set_CSN(0);
  OK=HAL_SPI_Transmit(&SPI_USE,(uint8_t *)&command,1,100);
  Set_CSN(1);
  
  command=Command(reg,1);
  Set_CSN(0);
  OK=HAL_SPI_TransmitReceive(&SPI_USE,(uint8_t *)&command,(uint8_t *)&AS_5047_Err,1,100);
  Set_CSN(1);
  
  command=Command(0x00,1);
  Set_CSN(0);
  OK=HAL_SPI_TransmitReceive(&SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);
  //OK=HAL_SPI_Receive(&SPI_USE,(uint8_t *)&result,1,100);
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

/* 
 get the rad or dgree position
 unit_type==0  rad
 else dgree
*/
float Get_Position_Rad_Dgree(uint8_t unit_type){
  uint16_t position=0;
  float result=0;
  position=Get_Position();
  if(unit_type==0){
    result=2*PI*position/16384.0f;
  }else{
    result=360.0f*position/16384.0f;
  }
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
  HAL_SPI_Transmit(&SPI_USE,(uint8_t *)&command,1,100);
  Set_CSN(1);

  command=Command(0x18,0);
  Set_CSN(0);
  HAL_SPI_TransmitReceive(&SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   // 接收错误寄存器
  Set_CSN(1);

  direction<<=2;
  command=Command(direction,0);
  Set_CSN(0);
  HAL_SPI_TransmitReceive(&SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   // 接收寄存器原值
  Set_CSN(1);

  command=Command(0x00,1);
  Set_CSN(0);
  HAL_SPI_TransmitReceive(&SPI_USE,(uint8_t *)&command,(uint8_t *)&result,1,100);   /// 发送NOP，接收寄存器新值
  Set_CSN(1);
}
