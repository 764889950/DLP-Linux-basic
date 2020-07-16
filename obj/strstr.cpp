/*****************************************************************************
* | File        :   strstr.cpp
* | Author      :   Youhua Lin
* | Function    :   Hardware underlying interface
* | Info        :
*                添加自定义字符处理函数，用于非极度标准的函数定义
*----------------
* |	This version:   V1.0
* | Date        :   2020-7-3
* | Info        :   Basic version
*
******************************************************************************/

#include <stdio.h>
#include <iconv.h>
#include <string.h>
#include <ctype.h>
#include "strstr.h"

void reboot(void)
{
	FILE* fp;
	fp=popen("sudo reboot now","r");//重启，同过管道读到fp 
	pclose(fp);
}


/*-------------------------------------------------------------------------------------
	'函数名:my_strstr
    '功能说明
    '-------------------------------------------------------------------------------------
    '简述:判断str2的前len_str个是否包含于str1
    '引用方法:重写strstr函数，消除结束标识影响
    '解释:------
    '=====================================================================================
    '参数说明
    '-------------------------------------------------------------------------------------
    '传入参数:
    '参数1       str1 第一个要比较的数据

    '函数参数:
    '参数2       str2 第二个要比较的数据
	
    '函数参数:
    '参数2       len_str 比较的数据长度
	'=====================================================================================
    '返回参数
    '解释: 返回NULL表示两个数据不相等，否则返回相同部分处str1的对应数据的首地址
	'=====================================================================================	
*/
char* my_strstr(const char *str1, const char *str2,int len_str)//str2为字符串子集，要参与的字符串,len_str1为传入要比较的长度，最大为str1的长度
{
	char *cp = (char *)str1;
	char *s1, *s2;
	
	int len=0;
	
	//注释以下两行，首字符为'\0'字符函数不直接不返回
	// if (!*str2)
		// return((char *)str1);
	
	//注释以下一行，改为判断长度变为str1的长度
	// while (*cp)
	while (len<len_str)
	{
		s1 = cp;
		s2 = (char *)str2;
		
		// while (*s2 && !(*s1 - *s2))
		while (*s2 && !(*s1 - *s2))
		{
			s1++,s2++;
			
		}
		
		if (!*s2)//s2的尾字符为'\0'时，表示str2包含于str1
			return(cp);

		cp++;
		
		len++;
		
	}
	
	return(NULL);
}



//c++11 no longer supplies a strcasecmp, so define our own version.
int stringcasecompare(const char* a, const char* b)
{
    while(*a && *b)
    {
        if (tolower(*a) != tolower(*b))
            return tolower(*a) - tolower(*b);
        a++;
        b++;
    }
    return *a - *b;
}


int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,  
        char *outbuf, size_t outlen) {  
    iconv_t cd;  
    char **pin = &inbuf;  
    char **pout = &outbuf;  
  
    cd = iconv_open(to_charset, from_charset);  
    if (cd == 0)  
        return -1;  
    memset(outbuf, 0, outlen);  
    if (iconv(cd, pin, &inlen, pout, &outlen) == ((size_t)(-1)))  
        return -1;  
    iconv_close(cd);  
    *pout = (char *)'\0';  
  
    return 0;  
}  

//字符串编码互转
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {  
    return code_convert((char*)"utf-8", (char*)"gb2312", inbuf, inlen, outbuf, outlen);  
}  

// int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {  
    // return code_convert((char*)"gb2312",(char*)"utf-8", inbuf, inlen, outbuf, outlen);  
// } 




