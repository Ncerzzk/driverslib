#include "pid.h"
#include "math.h"

#define Limit(value,max)     if(value>max)value=max;else if(value<-max)value=-max
float PID_Control(PID_S *PID,float target,float now){
  float err;
  float err_dt;
  float result;
  
  err=target-now;
  
  err_dt=err-PID->last_err;
  
  
  err_dt*=0.384f;
  err_dt+=PID->last_d*0.615f;  //低通滤波 
  
  
  PID->last_err=err;
  if(fabsf(err)<PID->I_ERR_LIMIT){
    PID->i+=err*PID->I_TIME;
  } 
  
  
  Limit(PID->i,PID->i_max);
  PID->last_d=err_dt;
  
  result = err * PID->KP  +   err_dt * PID->KD   +   PID->i * PID->KI;
  return result;
}

void reset_PID(PID_S * s){
  s->i=0;
  s->last_err=0;
  s->last_d=0;
}
