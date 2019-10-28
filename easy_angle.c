#include "easy_angle.h"
#include "math.h"

float gyro_k[3]={0.97,0.97,0.9};

void get_angle(int16_t * ac,float * angle_speed,float * angle,float *ac_angle){
  float R=0;
  float delta[3];
  if(!ac){
    return ;
  }
  
  R=sqrtf(((float)ac[0]*ac[0]+ac[1]*ac[1]+ac[2]*ac[2]));  //表达式中有float，会自动类型转化为float
  //原来的写法
  //R=sqrtf((float)(ac[0]*ac[0]+ac[1]*ac[1]+ac[2]*ac[2]));
  //有问题，当数比较大时，如0x7FFF，相乘之后会超过uint16_t范围，而转化为float是在溢出发生后，所以没用
  
  if(fabs(R)<1e-6f){
    return ;
  }
  for(int i=0;i<3;++i){
    ac_angle[i]=acosf((float)ac[i]/R)*180/3.14;
  }
  
  for(int i=0;i<3;++i){
    delta[i]=ac_angle[i]-angle[i];
  }

  /*
    需要注意，这里angle[0]到底是加上angle_speed[1]还是angle_speed[2]还是angle_speed[0]与板子
    的安装位置息息相关。
    
    另外，积分的符号，即angle[0]+=angle_speed，还是angle[0]-=angle_speed，也与安装有关
    如果angle_speed的方向与角度方向相反，这里就要用-的。
  */
  angle[0]+=delta[0]*(1-gyro_k[0])+angle_speed[1]*gyro_k[0]*0.005;
  angle[1]+=delta[1]*(1-gyro_k[1])-angle_speed[0]*gyro_k[1]*0.005;
  /* 偏航角计算
  float temp1=sin((angle[0]-90)/180.0f*3.14);
  float temp2=sin((angle[1]-90)/180.0f*3.14);
  
  float temp3=cos((angle[0]-90)/180.0f*3.14);
  float temp4=cos((angle[1]-90)/180.0f*3.14);
  angle[2]+=(angle_speed[0]*temp1+angle_speed[0]*temp2+\
  angle_speed[1]*temp1+angle_speed[1]*temp2+\
  angle_speed[2]*temp3+angle_speed[2]*temp4)*0.005;
  */
}