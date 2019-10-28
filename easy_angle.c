#include "easy_angle.h"
#include "math.h"

float gyro_k[3]={0.97,0.97,0.9};

void get_angle(int16_t * ac,float * angle_speed,float * angle,float *ac_angle){
  float R=0;
  float delta[3];
  if(!ac){
    return ;
  }
  
  R=sqrtf(((float)ac[0]*ac[0]+ac[1]*ac[1]+ac[2]*ac[2]));  //���ʽ����float�����Զ�����ת��Ϊfloat
  //ԭ����д��
  //R=sqrtf((float)(ac[0]*ac[0]+ac[1]*ac[1]+ac[2]*ac[2]));
  //�����⣬�����Ƚϴ�ʱ����0x7FFF�����֮��ᳬ��uint16_t��Χ����ת��Ϊfloat�����������������û��
  
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
    ��Ҫע�⣬����angle[0]�����Ǽ���angle_speed[1]����angle_speed[2]����angle_speed[0]�����
    �İ�װλ��ϢϢ��ء�
    
    ���⣬���ֵķ��ţ���angle[0]+=angle_speed������angle[0]-=angle_speed��Ҳ�밲װ�й�
    ���angle_speed�ķ�����Ƕȷ����෴�������Ҫ��-�ġ�
  */
  angle[0]+=delta[0]*(1-gyro_k[0])+angle_speed[1]*gyro_k[0]*0.005;
  angle[1]+=delta[1]*(1-gyro_k[1])-angle_speed[0]*gyro_k[1]*0.005;
  /* ƫ���Ǽ���
  float temp1=sin((angle[0]-90)/180.0f*3.14);
  float temp2=sin((angle[1]-90)/180.0f*3.14);
  
  float temp3=cos((angle[0]-90)/180.0f*3.14);
  float temp4=cos((angle[1]-90)/180.0f*3.14);
  angle[2]+=(angle_speed[0]*temp1+angle_speed[0]*temp2+\
  angle_speed[1]*temp1+angle_speed[1]*temp2+\
  angle_speed[2]*temp3+angle_speed[2]*temp4)*0.005;
  */
}