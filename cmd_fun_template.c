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


/*
将要增加的命令与函数写在这里
*/
void command_init(void){
  add_cmd("test",test);  
}


