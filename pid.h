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
    float I_TIME;    //2018年7月9日 修改，增加积分时间，
                    //以前是作为宏定义，但不同PID的积分应该是不一样的
}PID_S;

float PID_Control(PID_S *PID,float target,float now);
void reset_PID(PID_S * s);

#endif