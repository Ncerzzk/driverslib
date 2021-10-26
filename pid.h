#ifndef __PID_H
#define __PID_H

typedef struct{
	float KP;
	float KD;
	float KI;
	float i;
	float last_err;
	float i_max;
	float last_d;
    float I_TIME;    //2018��7��9�� �޸ģ����ӻ���ʱ�䣬
    float i_out_max; //��ǰ����Ϊ�궨�壬����ͬPID�Ļ���Ӧ���ǲ�һ����
	float I_ERR_LIMIT; // 积分限幅的阈值
}PID_S;

float PID_Control(PID_S *PID,float target,float now);
void reset_PID(PID_S * s);

#endif