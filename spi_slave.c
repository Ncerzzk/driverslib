#include "spi_slave.h"
#include "main.h"
#include "string.h"
// 采用8位通信
// 时序：
// 读： 主机：R|Reg(7bit)-----任意值------(读到寄存器值)
//      从机：8位状态值-------寄存器值----
// 写:  主机：W|Reg(7bit)-----寄存器新值----
//      从机：8位状态值-------寄存器原值-----(写入新值)

typedef enum{
    SPI_COM_IDLE,       // CSN未拉低之前
    SPI_COM_WaitSCK,    // CSN拉低之后，等待主机时钟
    SPI_COM_ReadFirstByte  // 收到第一个字节之后
    //SPI_COM_SendedVal   // 发送完值之后
}SPI_Com_Slave_Status;

typedef enum{
    SPI_COM_Read,
    SPI_COM_Write
}SPI_Com_Slave_Mode;

static SPI_Com_Slave_Status Status;
static SPI_HandleTypeDef *SPI_Use;
static uint8_t Rx_Buffer[10];
static SPI_Com_Slave_Mode Mode;
static uint16_t Reg_List_Length;
static uint8_t Now_Operate_Reg_Index;

extern SPI_COM_VAL SPI_Com_Val_List[];

inline void SPI_Slave_TransmitReceive(SPI_COM_VAL * com_val){
    HAL_SPI_TransmitReceive_DMA(SPI_Use,com_val->val_ptr,Rx_Buffer,com_val->val_size);
}
void SPI_Slave_CSN_Handler(){
    if(Status == SPI_COM_IDLE){
        Status = SPI_COM_WaitSCK;
        //HAL_SPI_TransmitReceive_DMA(SPI_Use,SPI_Com_Val_List[0].val_ptr,Rx_Buffer,SPI_Com_Val_List[0].val_size);
        SPI_Slave_TransmitReceive((SPI_COM_VAL* )SPI_Com_Val_List);
        // 这一句发送完毕后，会进入发送接收完成中断
    }
}

void SPI_Slave_Init(SPI_HandleTypeDef * hspi,uint16_t reg_list_len){
    SPI_Use=hspi;
    Reg_List_Length = reg_list_len;
}
void SPI_Slave_Normal_Hanlder(){
    switch (Status)
    {
    case SPI_COM_WaitSCK:
        Status=SPI_COM_ReadFirstByte;
        if(Rx_Buffer[0]&0x80){
            Mode = SPI_COM_Read;
        }else{
            Mode = SPI_COM_Write;
        }
        Now_Operate_Reg_Index = Rx_Buffer[0]&0x7F;
        if(Now_Operate_Reg_Index<Reg_List_Length){
            SPI_Slave_TransmitReceive(&SPI_Com_Val_List[Now_Operate_Reg_Index]);
        }else{
            // error!
            Status = SPI_COM_IDLE;
        }
        break;
    case SPI_COM_ReadFirstByte:
        Status = SPI_COM_IDLE;
        if(Mode==SPI_COM_Write){
            memcpy(SPI_Com_Val_List[Now_Operate_Reg_Index].val_ptr,Rx_Buffer,SPI_Com_Val_List[Now_Operate_Reg_Index].val_size);
        }
    break;
    default:
        break;
    }
    
}
