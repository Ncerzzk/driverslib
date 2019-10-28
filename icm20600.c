#include "icm20600.h"
//#include "usart.h"
#include "i2c.h"


MPU_Dev MPU9250;
MPU_Setting MPU9250_Setting;

/*
下面几个个函数需用户自己实现。
*/
extern I2C_HandleTypeDef hi2c1;
uint8_t I2C_Write_Buffer(uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len);
uint8_t I2C_Read_Buffer(uint8_t slaveAddr,uint8_t readAddr,uint8_t *pBuffer,uint16_t len);
void I2C_Reset(void);
extern void Delay_Us(uint32_t nus);



void mpu_error_deal(MPU_Dev *dev);
inline void mpu_error_deal(MPU_Dev *dev);
void I2C_Error_Check(MPU_Dev *dev,uint8_t i2c_result);

static void mpu_write_byte(MPU_Dev *dev,uint8_t write_addr,uint8_t data){
  if(dev->i2c_write_buffer(dev->dev_addr,write_addr,&data,1)!=dev->I2C_OK){
    mpu_error_deal(dev);
  }
}

inline void mpu_error_deal(MPU_Dev *dev){
  dev->i2c_error_count++;
  if(dev->i2c_error_count>20){
    dev->i2c_reset();
    dev->i2c_error_count=0;
  }
}

static uint8_t mpu_read_byte(MPU_Dev *dev,uint8_t read_addr){
  uint8_t temp=0;
  if(dev->i2c_read_buffer(dev->dev_addr,read_addr,&temp,1)!=dev->I2C_OK){
    mpu_error_deal(dev);
  }
  return temp;
}

inline void I2C_Error_Check(MPU_Dev *dev,uint8_t i2c_result){
  if(i2c_result!=dev->I2C_OK){
    mpu_error_deal(dev);
  }
}
void MPU_Read6500(MPU_Dev *dev,int16_t ac[],int16_t gy[]){
  
  uint8_t temp[6];
  uint8_t i2c_result;
  
  i2c_result=dev->i2c_read_buffer(dev->dev_addr,ACCEL_XOUT_H,temp,6);
  I2C_Error_Check(dev,i2c_result);
  ac[0]=(temp[0]<<8)|temp[1];
  ac[1]=(temp[2]<<8)|temp[3];
  ac[2]=(temp[4]<<8)|temp[5];
  
  i2c_result=dev->i2c_read_buffer(dev->dev_addr,GYRO_XOUT_H,temp,6);
  I2C_Error_Check(dev,i2c_result);
  gy[0]=(temp[0]<<8)|temp[1];
  gy[1]=(temp[2]<<8)|temp[3];
  gy[2]=(temp[4]<<8)|temp[5];	
}

void Gyroraw_to_Angle_Speed(MPU_Dev *dev,int16_t *gy,float *angle_speed ){
  for(int i=0;i<3;++i){
    angle_speed[i]=dev->setting->gyro_range*gy[i]/32768.0f;
  }
}



void MPU9250_Init(MPU_Dev * dev){
  
  dev->dev_addr=MPU_Address;
  dev->i2c_read_buffer=I2C_Read_Buffer;
  dev->i2c_write_buffer=I2C_Write_Buffer;
  dev->delay_ms=HAL_Delay;
  
  dev->I2C_OK=0x00;
  //dev->I2C_TIME_OUT=HAL_TIMEOUT;
  dev->i2c_reset=I2C_Reset;
  dev->delay_us=HAL_Delay;
  //dev->data=&MPU9250_Data;
  dev->setting=&MPU9250_Setting;
  
  dev->dev_mag_addr=MAG_Address;
  
  dev->setting->accel_range_setting=RANGE16G;
  dev->setting->accel_lpf_setting=0x01;//200hz
  dev->setting->gyro_range_setting=RANGE500;
  
MPU_INIT:
  
  dev->delay_ms(50);
  

  mpu_write_byte(dev,PWR_MGMT_1,0x01);
  mpu_write_byte(dev,SMPLRT_DIV, 0x00);
  mpu_write_byte(dev,MPU_CONFIG, 0x02);  //之前延时为20ms(0x06，现在为3ms左右 0x02)
  
  mpu_write_byte(dev,GYRO_CONFIG, dev->setting->gyro_range_setting);   //
  mpu_write_byte(dev,ACCEL_CONFIG, dev->setting->accel_range_setting); 
  mpu_write_byte(dev,ACCEL_CONFIG2, dev->setting->accel_lpf_setting); 
  
  //mpu_write_byte(dev,FIFO_ENABLE, GYRO_FIFO_EN|ACCEL_FIFO_EN); 
  
  dev->setting->mag_range=4912.0/100.0f; 
  
  mpu_write_byte(dev,INT_PIN_CFG,0x02);    //MPU6500 开启路过模式
  
  dev->delay_ms(50);
  
  
  
  switch(dev->setting->gyro_range_setting){
  case RANGE250:
    dev->setting->gyro_range=250.0f;
    break;
  case RANGE500:
    dev->setting->gyro_range=500.0f;
    break;
  case RANGE1000:
    dev->setting->gyro_range=1000.0f;
    break;
  case RANGE2000:
    dev->setting->gyro_range=2000.0f;
    break;
  }
  
  switch(dev->setting->accel_range_setting){
  case RANGE2G:
    dev->setting->accel_range=2;
    break;
  case RANGE4G:
    dev->setting->accel_range=4;
    break;	
  case RANGE8G:
    dev->setting->accel_range=8;
    break;	
  case RANGE16G:
    dev->setting->accel_range=16;
    break;		
  }
  
  
  
  dev->dev_ID=mpu_read_byte(dev,WHO_AM_I);
  
  if(dev->dev_ID!=0x11){
    dev->i2c_reset();
    //uprintf("icm20600 init failed!\r\n");
    goto MPU_INIT;
  }
  
}