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
static uint8_t Rx_Buffer[30];
static SPI_Com_Slave_Mode Mode;
static uint16_t Reg_List_Length;
static uint8_t Now_Operate_Reg_Index;

extern SPI_COM_VAL SPI_Com_Val_List[];

void SPI_Slave_TransmitReceive(SPI_COM_VAL * com_val){
    HAL_SPI_TransmitReceive_DMA(SPI_Use,(uint8_t *)com_val->val_ptr,(uint8_t *)Rx_Buffer,com_val->val_size);
}
void SPI_Slave_CSN_Handler(uint8_t flag){
    if(flag==0){
        Status = SPI_COM_WaitSCK;
        //HAL_SPI_TransmitReceive_DMA(SPI_Use,SPI_Com_Val_List[0].val_ptr,Rx_Buffer,SPI_Com_Val_List[0].val_size);
        SPI_Slave_TransmitReceive((SPI_COM_VAL* )SPI_Com_Val_List);
        // 这一句发送完毕后，会进入发送接收完成中断
    }else{
        HAL_SPI_Abort(SPI_Use);   // 正常流程下，该函数会很快结束。只有在SPI转串口模式下，会出现强制停止SPI_DMA的情况。
        Status = SPI_COM_IDLE;
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

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(hspi==SPI_Use){
        SPI_Slave_Normal_Hanlder();
    }
}


/*
-------------------    Master Interface     -------------------------
*/

#define Master_Delay(master)  master->Delay_Us_Func(10)
 
void SPI_COM_Master_Init(SPI_Com_Master * master,SPI_HandleTypeDef * master_hspi,GPIO_TypeDef* csn_port, uint16_t csn_pin,void (*delay_us_func) (uint16_t)){
    master->SPI_Use = master_hspi;
    master->Delay_Us_Func = delay_us_func;
    master->CSN_Port = csn_port;
    master->CSN_Pin = csn_pin;
}

void SPI_COM_Master_Read_Reg(SPI_Com_Master * master,uint8_t reg_addr,uint8_t * reg_data,uint16_t size){
    uint8_t temp=0;
    temp = reg_addr | 0x80;
    HAL_GPIO_WritePin(master->CSN_Port,master->CSN_Pin,GPIO_PIN_RESET);
    Master_Delay(master);
    HAL_SPI_TransmitReceive(master->SPI_Use,&temp,master->Rx_Buffer,1,10); // 此时Master_Rx_Buffer里存的应该是List[0]里的寄存器的值，8位
    Master_Delay(master);
    HAL_SPI_TransmitReceive(master->SPI_Use,master->Rx_Buffer,master->Rx_Buffer,size,10);
    Master_Delay(master);
    HAL_GPIO_WritePin(master->CSN_Port,master->CSN_Pin,GPIO_PIN_SET);
    memcpy(reg_data,master->Rx_Buffer,size);

    Master_Delay(master);
}

void SPI_COM_Master_Write_Reg(SPI_Com_Master * master,uint8_t reg_addr,uint8_t * reg_data,uint16_t size){
    uint8_t temp=0;
    temp = reg_addr;
    HAL_GPIO_WritePin(master->CSN_Port,master->CSN_Pin,GPIO_PIN_RESET);
    Master_Delay(master);
    HAL_SPI_TransmitReceive(master->SPI_Use,&temp,master->Rx_Buffer,1,10); //此时Master_Rx_Buffer里存的应该是List[0]里的寄存器的值，8位
    Master_Delay(master);
    HAL_SPI_TransmitReceive(master->SPI_Use,reg_data,master->Rx_Buffer,size,10);
    Master_Delay(master); 
    HAL_GPIO_WritePin(master->CSN_Port,master->CSN_Pin,GPIO_PIN_SET);
    Master_Delay(master);
}

