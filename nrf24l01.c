#include "nrf24l01.h"
#include "stdlib.h"
#include "string.h"




extern SPI_HandleTypeDef hspi3;

SPI_HandleTypeDef * SPI_USE;


#define NRF_SPI_Write_Read(write_ptr,read_ptr,len)   HAL_SPI_TransmitReceive(SPI_USE,write_ptr,read_ptr,len,50) 
#define NRF_SPI_Read(read_ptr,len) HAL_SPI_Receive(SPI_USE,read_ptr,len,50)
#define NRF_SPI_Write(write_ptr,len) HAL_SPI_Transmit(SPI_USE,write_ptr,len,50)

#define CRC_ENABLE   1



uint8_t  RX_ADDRESS[3][5]= {{0x20,0x30,0x40,0x40,0x42},
                            {0x1,0x2,0x3,0x4,0x5},
                            }; 
uint8_t  TX_ADDRESS[5]= {0x20,0x30,0x40,0x40,0x42}; 



uint8_t  NRF_TX_Data[TX_PLOAD_WIDTH]={1,2,3,4,5,6,7,8,9,10};
uint8_t  NRF_RX_Data[RX_PLOAD_WIDTH]={0};


uint8_t STATUS_VALUE;

uint8_t CONFIG_VALUE=0x00;  // Enable all Interupts

GPIO_TypeDef *  CSN_GPIO_PORT;
GPIO_TypeDef * CE_GPIO_PORT;
uint32_t CSN_GPIO_PIN;
uint32_t CE_GPIO_PIN;

static void NRF_Set_GPIO(NRF_GPIO io,NRF_GPIO_Level level){
  if(io==CSN){
    HAL_GPIO_WritePin(CSN_GPIO_PORT,CSN_GPIO_PIN,level);
  }else if(io==CE){
    HAL_GPIO_WritePin(CE_GPIO_PORT,CE_GPIO_PIN,level);
  } 
}

uint8_t NRF_Read_Reg(uint8_t reg){
  uint8_t rxData=0;
  uint8_t nop=0xFF;
  NRF_Set_GPIO(CSN,LOW);
  NRF_SPI_Write_Read(&reg,&STATUS_VALUE,1);
  NRF_SPI_Write_Read(&nop,&rxData,1);

 // NRF_SPI_Read(&rxData,1);
  NRF_Set_GPIO(CSN,HIGH);
  return rxData;  
}

uint8_t NRF_Write_Reg(uint8_t reg,uint8_t data){
  uint8_t spi_send_data=W_REGISTER|reg;     
  // 对命令来说，除了读命令，其他的命令的0x20处都是1，所以无所谓
  // 可以用此函数来发送清除 或者别的命令
  
  NRF_Set_GPIO(CSN,LOW);
  NRF_SPI_Write_Read(&spi_send_data,&STATUS_VALUE,1);
  NRF_SPI_Write(&data,1);
  NRF_Set_GPIO(CSN,HIGH);

  return 1;
}

void NRF_Read_Bytes(uint8_t addr,uint8_t *rx_data,uint8_t len){
  uint8_t * nopdata=(uint8_t *) malloc(sizeof(uint8_t)*len);
  memset(nopdata,0xFF,len);
  NRF_Set_GPIO(CSN,LOW);
  NRF_SPI_Write_Read(&addr,&STATUS_VALUE,1);
  NRF_SPI_Write_Read(nopdata,rx_data,len);
  //NRF_SPI_Read(rx_data,len);
  NRF_Set_GPIO(CSN,HIGH);
  free(nopdata);
  
}

uint8_t NRF_Write_Bytes(uint8_t addr,uint8_t *data,uint8_t len){
  uint8_t * temp=(uint8_t *)malloc(sizeof(uint8_t)*len);
  uint8_t spi_send_data=W_REGISTER|addr;
  
  NRF_Set_GPIO(CSN,LOW);
  NRF_SPI_Write_Read(&spi_send_data,&STATUS_VALUE,1);
  NRF_SPI_Write_Read(data,temp,len);
  NRF_Set_GPIO(CSN,HIGH);
  
  NRF_Read_Bytes(addr,temp,len);  
  free(temp);
  return 1;
}



uint8_t NRF_Init(SPI_HandleTypeDef * spi_port, uint8_t rx_channel_num,\
                  GPIO_TypeDef * csn_gpio_port,uint32_t csn_gpio_pin,\
                  GPIO_TypeDef * ce_gpio_port,uint32_t ce_gpio_pin){
  
  uint8_t result=1;
  uint8_t retry_time=0;

  SPI_USE=spi_port;
  CSN_GPIO_PORT=csn_gpio_port;
  CE_GPIO_PORT=ce_gpio_port;
  CSN_GPIO_PIN=csn_gpio_pin;
  CE_GPIO_PIN=ce_gpio_pin;

REDO:
  retry_time++;
  if(retry_time>5){
    return 0;
  }
  result&=NRF_Write_Reg(NRF_CONFIG,CONFIG_VALUE|NRF_TX);   // Power Up

  HAL_Delay(10);
  result&=NRF_Write_Reg(EN_AA,0X01); //自动ACK
  result&=NRF_Write_Reg(SETUP_RETR,0x0F); //自动重发
  result&=NRF_Write_Reg(RF_CH,0); //2.4ghz
  result&=NRF_Write_Reg(RX_PW_P0,RX_PLOAD_WIDTH); 
  result&=NRF_Write_Reg(RF_SETUP,0x0E); //2mhz 0db
  
  if(rx_channel_num>5){
    return 0;

  }
  uint8_t temp=0;
  temp=(1<<rx_channel_num)-1;
  
  result&=NRF_Write_Reg(EN_RXADDR,temp);//允许通道n 接收
  result&=NRF_Write_Reg(NRF_CONFIG,CONFIG_VALUE|NRF_RX);     // 设置为接收模式
  
  result&=NRF_Set_Tx_Addr(TX_ADDRESS,sizeof(TX_ADDRESS));
  
  NRF_Set_Rx_Addr(RX_ADDRESS,sizeof(RX_ADDRESS));
  
  if(!result){
    result=1;
    goto REDO;
  }
  
  return result;
}

uint8_t NRF_Set_Tx_Addr(uint8_t * addr,size_t size_check){  
  // the lengh of the array must be 5
  // after set_tx_addr, must init the nrf  again!
  uint8_t result=0;
  if(size_check!=sizeof(TX_ADDRESS)){
    return 0;
  }
  result=NRF_Write_Bytes(TX_ADDR,addr,size_check);
  return result;
}

uint8_t NRF_Set_Rx_Addr(uint8_t rx_addrs[][5],size_t size_check){
   // the lengh of the array must be 5
  // after set_rx_addr, must init the nrf  again!
  
  /*
  the rx_addrs must be
  {addr0_byte1,addr0_byte2,addr0_byte3,addr0_bye4,addr0_byte5},
  {addr1_byte1,addr1_byte2,addr1_byte3,addr1_bye4,addr1_byte5},
  {addr2_byte5,addr3_byte5,addr4_byte5,addr5_byte5,0}
  */
  
  uint8_t result=1;
  if(size_check!=sizeof(RX_ADDRESS)){
    return 0;
  }
  
  for(int i=0;i<2;++i){
    result&=NRF_Write_Bytes(RX_ADDR_P0 + i,rx_addrs[i],sizeof(RX_ADDRESS[0]));
  }
  for(int i=0;i<4;++i){
    result&=NRF_Write_Reg(RX_ADDR_P0+2+i,rx_addrs[2][i]);
  }
  return result;
}

static void NRF_Set_Mode(NRF_Mode mode,uint8_t EN_CRC){
  if(EN_CRC){
    mode|=0x08;
  }
  CONFIG_VALUE|=mode;
  NRF_Write_Reg(NRF_CONFIG,CONFIG_VALUE);
}

void NRF_Enable_Interrupt(NRF_IT IT_Mask){
  CONFIG_VALUE&=~IT_Mask;
  NRF_Write_Reg(NRF_CONFIG,CONFIG_VALUE);
}

void NRF_Send_Message(){
  
  NRF_Set_Mode(NRF_TX,CRC_ENABLE); 
  NRF_Write_Bytes(W_TX_PAYLOAD,NRF_TX_Data,TX_PLOAD_WIDTH);
  NRF_Set_GPIO(CE,HIGH);
  
  do{
    STATUS_VALUE=NRF_Read_Reg(STATUS);
    if((STATUS_VALUE&0x10)==0x10){
      //NRF_Write_Reg(FLUSH_TX,0);
      NRF_Write_Reg(STATUS,0x10);  //if MAX_RT is asserted it must be cleared to enable further communication.
    }
  }while((STATUS_VALUE&0x20)==0);
  NRF_Send_Callback();
   
  NRF_Write_Reg(STATUS,0x20);
  NRF_Set_GPIO(CE,LOW);

}

uint8_t FIFO_State;
void NRF_Receive(){


  //
  NRF_Set_GPIO(CE,HIGH);
  NRF_Set_Mode(NRF_RX,CRC_ENABLE); 
  
  if(STATUS_VALUE<0x40){
    return ;
  }
  NRF_Set_GPIO(CE,LOW);
  // 这里只是接收定长数据，如果要接收动态长度，需要一堆操作
  // 以后有时间再说吧

  FIFO_State=NRF_Read_Reg(0x17);
  if(FIFO_State & 1<<1){
    NRF_Write_Reg(FLUSH_RX,0);
    NRF_Write_Reg(STATUS,0xFF);
    return ;
  }
  NRF_Read_Bytes(R_RX_PAYLOAD,NRF_RX_Data,RX_PLOAD_WIDTH);
  
  NRF_Receive_Callback(NRF_RX_Data,RX_PLOAD_WIDTH);
  NRF_Write_Reg(STATUS,0xFF);  //清空STATUS
}

void NRF_Install_TX_Data(uint8_t *data,int len){
  memcpy(NRF_TX_Data,data,len);
}
__weak void NRF_Receive_Callback(uint8_t * data,int len){
  
}

__weak void NRF_Send_Callback(){
  
}

void NRF_Send_Message_IT(){
  STATUS_VALUE=NRF_Read_Reg(STATUS);
  if((STATUS_VALUE&0x10)==0x10){
    // 超过最大重试次数了
    NRF_Write_Reg(STATUS,0x10);  //if MAX_RT is asserted it must be cleared to enable further communication.
  }
  
  NRF_Set_Mode(NRF_TX,CRC_ENABLE); 
  NRF_Write_Bytes(W_TX_PAYLOAD,NRF_TX_Data,TX_PLOAD_WIDTH);
  NRF_Set_GPIO(CE,HIGH);  
}

void NRF_Receive_IT(){
 NRF_Write_Reg(STATUS,0xFF);
 STATUS_VALUE=NRF_Read_Reg(STATUS);
 NRF_Write_Reg(FLUSH_RX,0);
 
 NRF_Set_Mode(NRF_RX,CRC_ENABLE); 
 NRF_Set_GPIO(CE,HIGH); 

}
void NRF_TX_IRQ_Handler(){
  STATUS_VALUE=NRF_Read_Reg(STATUS);
  if(STATUS_VALUE&0x20){
    NRF_Set_GPIO(CE,LOW);
  }
  NRF_Write_Reg(FLUSH_TX,0);    // 这一句似乎不需要
  NRF_Write_Reg(STATUS,0xFF);
}
#include "uart_ext.h"
void NRF_RX_IRQ_Handler(){
  NRF_Read_Bytes(R_RX_PAYLOAD,NRF_RX_Data,RX_PLOAD_WIDTH);

  NRF_Set_GPIO(CE,LOW);

  NRF_Write_Reg(STATUS,0xFF);  //清空STATUS
 
  NRF_Receive_Callback(NRF_RX_Data,RX_PLOAD_WIDTH);  
  NRF_Write_Reg(FLUSH_RX,0); 
}

