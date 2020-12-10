#include "arm_math.h"
#define Semig 1e-6f
#define INTEGRAL_CONSTANT 0.002f		//积分时间 2ms
#define HALF_T 0.001f


float q0=1.0f;
float q1=0.0f;
float q2=0.0f;
float q3=0.0f;

float exInt,eyInt,ezInt;

float IMU_P=1.0f;				//2.0f   //20
float IMU_I=0.005f;		//0.005          0.03

void IMU_Update(float * ac,float * gy,float *attitude,float *ace){

	float ax,ay,az,wx,wy,wz;
    float q0q0 = q0 * q0;                                                        
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q1q1 = q1 * q1;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;
	float q1q2=q1*q2;
	float q0q3=q0*q3;
	float norm;
    float gbx,gby,gbz;
    float ex,ey,ez;


    ax=ac[0];ay=ac[1];az=ac[2];
    wx=gy[0];wy=gy[1];wz=gy[2];
      
    //计算加速度旋转到天地坐标系的值
    ace[0] = 2*ax*(0.5f - q2q2 - q3q3) + 2*ay*(q1q2 - q0q3) + 2*az*(q1q3 + q0q2); 
	ace[1] = 2*ax*(q1q2 + q0q3) + 2*ay*(0.5f - q1q1 - q3q3) + 2*az*(q2q3 - q0q1); 
	ace[2] = 2*ax*(q1q3 - q0q2) + 2*ay*(q2q3 + q0q1) + 2*az*(0.5f - q1q1 - q2q2);
    
	arm_sqrt_f32(ax*ax+ay*ay+az*az,&norm);
    
	if(norm<Semig)
		return ;
	ax/=norm;
	ay/=norm;
	az/=norm; 
	
	//计算重力加速度旋转到机体坐标系后的值,即以当前估计的姿态作为旋转矩阵
	gbx= 2*(q1q3 - q0q2);
	gby= 2*(q0q1 + q2q3);
	gbz= q0q0 - q1q1 - q2q2 + q3q3;
	
	//与实际加速度计测得的ax,ay,az做叉积，取误差
    ex = (ay*gbz - az*gby);                                                                
    ey = (az*gbx - ax*gbz);
    ez = (ax*gby - ay*gbx);	
	
	
    exInt += ex*IMU_I*INTEGRAL_CONSTANT;
    eyInt += ey*IMU_I*INTEGRAL_CONSTANT;
    ezInt += ez*IMU_I*INTEGRAL_CONSTANT;
    
    //补偿误差
    wx+=ex*IMU_P+exInt;
    wy+=ey*IMU_P+eyInt;
    wz+=ez*IMU_P+ezInt;
    
    
    //更新四元数
    q0=  q0 + (-q1*wx - q2*wy - q3*wz)*HALF_T;
    q1 = q1 + (q0*wx + q2*wz - q3*wy)*HALF_T;
    q2 = q2 + (q0*wy - q1*wz + q3*wx)*HALF_T;
    q3 = q3 + (q0*wz + q1*wy - q2*wx)*HALF_T; 
	
    //计算欧拉角
    arm_sqrt_f32(q0*q0+q1*q1+q2*q2+q3*q3,&norm);
    if(norm<Semig)
      return ;
    q0/=norm;
    q1/=norm;
    q2/=norm;
    q3/=norm;
    attitude[0]= asinf(-2*q1*q3 + 2*q0*q2)* 57.3f;           //pitch
    attitude[1]= atan2f(2*q2*q3 + 2*q0*q1, -2*q1*q1 - 2*q2*q2+1)* 57.3f;   //roll
    attitude[2]= atan2f(2*q1q2 + 2*q0q3, -2*q2q2 - 2*q3q3+1)* 57.3f;         //yaw
}