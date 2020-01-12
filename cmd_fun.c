#include "cmd_fun.h"
#include "command.h"
#include "uart_ext.h"


typedef struct{
  char * var_name;
  void * value_ptr;
}Var_Edit_Struct;

Var_Edit_Struct Var_List[10]={
  //{"first",&First_Time_Check}
  0
};

static void test(int arg_num,char **string_prams,float * arg){
  int i;
  uprintf("this is testkk\r\n");
  uprintf("i get %d args\r\n",arg_num);
  for(i=0;i<(arg_num&0xFF);++i){
    uprintf("one is %f\r\n",arg[i]);
  }
  for(i=0;i<(arg_num>>8);++i){
    uprintf("string_prams is %s\r\n",string_prams[i]);
  }
}

void set_val(int arg_num,char ** s,float * args){
  void * edit_value;
  if(arg_num!=0x0201){
    uprintf("error arg_num!\r\n");
    return ;
  }

  for(int i=0;i<sizeof(Var_List)/sizeof(Var_Edit_Struct);++i){
    if(compare_string(Var_List[i].var_name,s[0])){
      edit_value=Var_List[i].value_ptr;
      break;
    }
  }
  
  if(compare_string(s[1],"u8")){
    *(uint8_t *)edit_value=(uint8_t)args[0];
    uprintf("ok set %s = %d\r\n",s[0],*(uint8_t *)edit_value);  
  }else if(compare_string(s[1],"int")){
    *(int16_t *)edit_value=(int16_t)args[0];
    uprintf("ok set %s = %d\r\n",s[0],*(int16_t *)edit_value);
  }else if(compare_string(s[1],"f")){
    *(float *)edit_value=args[0];
    uprintf("ok set %s = %f\r\n",s[0],*(float *)edit_value);
  }
}




/*
下面定义用户需要的函数

*/

#include "tim.h"

void TIM_test(int arg_num,char ** s,float * args){
  if(arg_num!=0x0003 && arg_num!=0x0002 && arg_num!=0x0001){
    uprintf("error argnum!\r\n");
    return ;
  }
  
  if(arg_num==0x0001){
    TIM3->CCR3=Caculate_Cnt(&htim3,64,(int)args[0]);
    TIM3->CCR4=Caculate_Cnt(&htim3,64,(int)args[0]);
    uprintf("ok,set width =%d \r\n",(int)args[0]);
  }else if(arg_num==0x0002){
    if((int)args[0]==2){
      TIM2->CCR1=Caculate_Cnt(&htim2,64,(int)args[1]);
      TIM2->CCR2=Caculate_Cnt(&htim2,64,(int)args[1]);     
    }else{
      TIM3->CCR3=Caculate_Cnt(&htim3,64,(int)args[1]);
      TIM3->CCR4=Caculate_Cnt(&htim3,64,(int)args[1]);
    }
  }else if(arg_num==0x0003){
    // args[0] TIM2 TIM3
    // args[1] CH1 CH2 CH3 CH4
    // args[2] pulse width
    volatile uint32_t * ptr=0;
    int32_t cnt=0;
    if((int)args[0]==2){
      ptr=&(TIM2->CCR1);
      cnt=Caculate_Cnt(&htim2,64,(int)args[2]); 
    }else{
      ptr=&(TIM3->CCR1);
      cnt=Caculate_Cnt(&htim3,64,(int)args[2]); 
    }
    ptr+=(int)args[1];
    *ptr=cnt;
  }
  
}

#include "control.h"
void servor_test(int arg_num,char ** s,float * args){
  if(arg_num!=0x0001&&arg_num!=0x0101){
    uprintf("error arg_num!\r\n");
    return ;
  }
  if(arg_num==0x0001){
    Set_Servor(L_Servor,args[0]);
    Set_Servor(R_Servor,args[0]);
  }
}

void esc_test(int arg_num,char ** s,float * args){
  if(arg_num!=0x0001&&arg_num!=0x0101){
    uprintf("error arg_num!\r\n");
    return ;
  }
  if(arg_num==0x0001){
    Set_ESC(L_ESC,args[0]);
    Set_ESC(R_ESC,args[0]);
  }else{
    if(s[0][0]=='l'){
      Set_ESC(L_ESC,args[0]);
    }else{
      Set_ESC(R_ESC,args[0]);
    }
  }
}

/*
将要增加的命令与函数写在这里
*/
void command_init(void){
  add_cmd("test",test);  
  add_cmd("tim_test",TIM_test);
  add_cmd("ser_test",servor_test);
  add_cmd("esc_test",esc_test);
}


