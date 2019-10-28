#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "usart.h"
#include "ctype.h"
#include "uart_ext.h"


struct {
  char * array[CMD_MAX_NUM];
  int array_ptr;
  cmdFunc func_array[CMD_MAX_NUM];
}CMD;




#define STRING_PRAM_NUM 4
#define FLOAT_PRAM_NUM  4

static int16_t get_prams(char *s,char *** string_prams,float ** float_prams){
  int i=0;
  enum {IN_DIGITAL=1,IN_LETTER=2,IN_SPACE=0}in_flag;
  char **float_buffer;
  char **string_buffer;
  
  int num_float_i=0,num_string_i=0; //�ڼ�������
  int num_i_i=0;//�ڼ�������/�ַ����ĵڼ�λ
  
  in_flag=IN_SPACE;
  
  /*
  ���ֲ����Ĵ��λ�ã���Щ�ռ���Ҫ�ں������ͷ�
  */
  *float_prams=(float *)malloc(sizeof(int)*FLOAT_PRAM_NUM);
  
  /*
  ���ֲ������ı���ʱ��ţ����ڵ���atof����
  �ڱ��������ǰ�Ὣ��Щ�ռ��ͷŵ�
  */
  float_buffer=(char **)malloc(sizeof(char *)*FLOAT_PRAM_NUM);
  for(i=0;i<FLOAT_PRAM_NUM;++i){
    float_buffer[i]=(char *)malloc(sizeof(char)*20);
  }
  
  /*
  �ַ��������Ĵ��λ�ã���Щ�ռ���Ҫ�ں������ͷ�
  */
  string_buffer=(char **)malloc(sizeof(char*) * STRING_PRAM_NUM);
  for(i=0;i<STRING_PRAM_NUM;++i){
    string_buffer[i]=(char *)malloc(sizeof(char)*20);
  }
  
  *string_prams=string_buffer;
  
  i=0;
  //ʹ��DMA������s[1]!='\r'����������Ϊû���ٰ�\r����\0��
  while(s[i]!='\0'&&s[i]!='\r'&&num_float_i<=FLOAT_PRAM_NUM&&num_string_i<=STRING_PRAM_NUM&&num_i_i<20){
    if(s[i]!=' '){//����ǿո�
      if(in_flag==IN_SPACE){  //�������б�־λ���ַ�����־λδ����,���ӿո�֮����ֵĵ�һ������/�ַ�
        if(isalpha(s[i])){
          in_flag=IN_LETTER;
          num_string_i++;
          string_buffer[num_string_i-1][0]=s[i];  //���Ƶ�һ���ַ�
        }else{
          in_flag=IN_DIGITAL;
          num_float_i++;
          float_buffer[num_float_i-1][0]=s[i];   //���Ƶ�һ�������ַ�
        }
        num_i_i=0;
      }else{
        //��������
        if(in_flag==IN_LETTER){
          string_buffer[num_string_i-1][num_i_i]=s[i];
        }else{
          float_buffer[num_float_i-1][num_i_i]=s[i];
        }
      }
      num_i_i++;
    }else{ //�ǿո�
      if(in_flag==IN_LETTER){
        string_buffer[num_string_i-1][num_i_i]='\0';
      }else if(in_flag==IN_DIGITAL){
        float_buffer[num_float_i-1][num_i_i]='\0';
      }
      in_flag=IN_SPACE;
    }
    i++;
  }
  if(in_flag==IN_LETTER){  //������ַ����������������������ټ��һ�£�����\0
    string_buffer[num_string_i-1][num_i_i]='\0';
  }else if(in_flag==IN_DIGITAL){
    float_buffer[num_float_i-1][num_i_i]='\0';
  }
  
  /*
  �ͷ����ֲ������ַ���ʱ��ſռ�
  */
  for(i=0;i<FLOAT_PRAM_NUM;++i){
    (*float_prams)[i]=atof(float_buffer[i]);
    free(float_buffer[i]);
  }
  free(float_buffer);
  
  return num_string_i<<8|num_float_i;  
}


/*
void add_cmd(const char * s,cmdFunc f)
�������sΪ�����ַ�����fΪ�󶨵ĺ�����
*/


void add_cmd(char * s,cmdFunc f){
  if(CMD.array_ptr>CMD_MAX_NUM-1)
    return ;
  CMD.array[CMD.array_ptr]=s;
  CMD.func_array[CMD.array_ptr]=f;
  CMD.array_ptr++;
}

/*
compare_cmd(const char* cmd,char *s)
�Ƚ�cmd��s�����ַ���������cmdΪ���sΪ���Ƚϵ��ַ���
���ַ���Ϊ������򷵻�����ĳ��ȣ����򷵻�0
*/
char compare_cmd(const char * cmd,char * s){
  int i=0;
  while(cmd[i]!='\0'&&s[i]!='\0'){
    if(cmd[i]!=s[i])
      return 0;
    ++i;
  }
  if(s[i]!=' '&&s[i]!='\0'&&s[i]!='\r'){          //ʹ��DMA�����ж����һ��i�׼�
    return 0;   //��������hello ������ helloworld�������û������жϣ���ɺ�������Ϊǰ��
  }
  return i;
}

uint8_t compare_string(const char *s1,char * s2){
  int i=0;
  while(s1[i]==s2[i]&&s1[i]&&s2[i]){
    i++;
  }
  if(!s1[i]&&!s2[i]){
    return 1;
  }else{
    return 0;
  }  
}

void analize(uint8_t *s){
  int i=0;
  int j=0,temp=0;
  int prams_num;
  char ** string_prams;
  float * float_prams;
  for(i=0;i<CMD.array_ptr;++i){
    j=compare_cmd(CMD.array[i],(char*)s);
    if(j){
      prams_num=get_prams((char*)s+j,&string_prams,&float_prams);
      CMD.func_array[i](prams_num,string_prams,float_prams);
      free(float_prams);
      for(temp=0;temp<STRING_PRAM_NUM;++temp){
        free(string_prams[temp]);
      }
      free(string_prams);
      return ;
    }
  }
  uprintf("error command!\r\n");
}


