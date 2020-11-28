#ifndef __ICM20600_H
#define __ICM20600_H

#include "stdint.h"

typedef uint8_t (*i2c_write_buffer_fptr)(uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len);
typedef uint8_t (*i2c_read_buffer_fptr) (uint8_t slaveAddr, uint8_t writeAddr, uint8_t *pBuffer,uint16_t len);
typedef void (*i2c_err_reset_fptr) (void);
typedef void (*delay_ms_fptr)(uint32_t t);
typedef void (*delay_us_fptr)(uint32_t t);


#define	SMPLRT_DIV		0x19	//???????????????????0x01(500Hz)
//???????=????????????/(1+SMPLRT_DIV )    ?????????????????????????????8KHZ???????????????1K

#define	MPU_CONFIG			0x1A	//?????????????????0x01(188Hz)
#define	GYRO_CONFIG		0x1B	//???????????????��?????????0x18(?????2000deg/s)
#define	ACCEL_CONFIG	0x1C	//??????????????��???????????????????0x01(?????2G??5Hz)
#define ACCEL_CONFIG2   0x1D                    // ICM20600????
#define FIFO_ENABLE     0x23                    //ICM20600????
  #define GYRO_FIFO_EN    0x10
  #define ACCEL_FIFO_EN  0x08 
#define INT_PIN_CFG		0x37
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B	//????????????????0x00(????????)
#define	WHO_AM_I	    0x75	//IIC????????(??????0x68?????)

#define GYRO_Range_Configure 0x8   //0x10
#define GYRO_Range 500

#define ACCEL_Range_Configure 0x1  
#define ACCEL_Range 2*9.8f



#define MPU_Address   0xD0
#define MAG_Address 	0x18          //0x0C?????��

#define MAG_XOUT_L		0x03
#define MAG_XOUT_H		0x04
#define MAG_YOUT_L		0x05
#define MAG_YOUT_H		0x06
#define MAG_ZOUT_L		0x07
#define MAG_ZOUT_H		0x08

#define MAG_CONTROL 	0x0A
#define MAG_CONTROL2    0x0B

#define MAG_ST1             0x02
#define MAG_ST2             0x09

#define MAG_ASAX			0x10
#define MAG_ASAY			0x11
#define MAG_ASAZ			0x12

#define MAG_WIA             0x00




typedef enum{
	RANGE250=0x00,    //+-250
	RANGE500=0x08,
	RANGE1000=0x10,
	RANGE2000=0x18
}GYRO_RANGE;

typedef enum{
	RANGE2G=0x00,
	RANGE4G=0x08,
	RANGE8G=0x10,
	RANGE16G=0x18
}ACCEL_RANGE;

typedef struct{
	GYRO_RANGE gyro_range_setting;
	ACCEL_RANGE accel_range_setting;
	uint8_t accel_lpf_setting;
	float gyro_range;
	int16_t accel_range;
    float mag_range;
}MPU_Setting;

typedef struct{
	i2c_write_buffer_fptr i2c_write_buffer;
	i2c_read_buffer_fptr i2c_read_buffer;
	i2c_err_reset_fptr i2c_reset;
	delay_ms_fptr delay_ms;
        delay_us_fptr delay_us;
    
	uint8_t dev_addr;  //8��???????7��????????��?????
	uint8_t dev_mag_addr; //????????
	uint8_t dev_ID;
	uint8_t I2C_OK;
//    uint8_t I2C_TIME_OUT;
	uint16_t i2c_error_count;
	MPU_Setting *setting;
}MPU_Dev;

void MPU9250_Init(MPU_Dev * dev);

extern MPU_Dev MPU9250;

void MPU_Read6500(MPU_Dev *dev,int16_t *,int16_t *);
void Gyroraw_to_Angle_Speed(MPU_Dev *dev,int16_t  gy[3],float angle_speed[3]);
#endif
