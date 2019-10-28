#ifndef COMMAND_H
#define COMMAND_H

#include "stdint.h"

#define CMD_MAX_NUM 50

typedef void (*cmdFunc)(int arg_num,char ** string_arg,float * float_arg);


void add_cmd(char * s,cmdFunc f);
void analize(uint8_t * s);

char compare_cmd(const char * cmd,char * s);
uint8_t compare_string(const char *s1,char * s2);

#endif
