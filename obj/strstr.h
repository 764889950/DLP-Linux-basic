/*****************************************************************************
* | File        :   Debug.h
* | Author      :   Waveshare team
* | Function    :   debug with printf
* | Info        :
*----------------
* | This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <stdio.h>

void reboot(void);
char* my_strstr(const char *str1, const char *str2,int len_str);
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

#endif

