#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strstr.h"

#define Data_SIZE 300
char buf_recv[Data_SIZE],buf_send[Data_SIZE]; //套接字发送接收变量 

void socket_send(const char buf_send[])
{
    int sockfd;  
    int len;  
    struct sockaddr_in address;  
    int result;  

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)  
    {  
        perror("socket");  
        exit(EXIT_FAILURE);  
    }  
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 8080;  
    len = sizeof(address);  
    if((result = connect(sockfd,(struct sockaddr *)&address,len)) == -1)  
    {  
        perror("connect");  
        exit(EXIT_FAILURE);  
    }  
  
    printf("please enter the context you want to send to server :"); 

/*
套接字send函数说明
（1）第一个参数指定发送端套接字描述符；

（2）第二个参数指明一个存放应用程序要发送数据的缓冲区；

（3）第三个参数指明实际要发送的数据的字节数；

（4）第四个参数一般置0。
*/
	if(send(sockfd,buf_send,Data_SIZE,0) == -1)  
	{  
		perror("send");  
		exit(EXIT_FAILURE);  
	}
	
	if(recv(sockfd,buf_recv,Data_SIZE,0) == -1)  
	{  
		perror("recv");  
		exit(EXIT_FAILURE);  
	} 
	printf("receive from server %s\n",buf_recv);  
      
    close(sockfd);  
    exit(0);  
}

//c++11 no longer supplies a strcasecmp, so define our own version.
static int stringcasecompare(const char* a, const char* b)
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

void print_usage()
{
    printf("\n");
    printf("usage:\n");
    printf("control-client help\n");
    printf("\tShow this help message\n");
    printf("\n");
    printf("control-client slice [-p <file_print.slc>] [-j <settings.json>] [-r <setting>] [-s <settingkey>=<value>]\n");
    printf("  -p\n\topen a 3D model.\n");
    printf("  -j\n\tLoad settings.def.json file to register all settings and their defaults.\n");
	printf("  -r\n\tcontinue or close a 3D model.\n");
    printf("  -s <setting>=<value>\n\tSet a setting to a value\n");
    printf("\n");

}

void slice(int argc, char **argv)
{
	char open_print_file[300]="open_print_file:";
    for(int argn = 2; argn < argc; argn++)
    {
        char* str = argv[argn];
        if (str[0] == '-')
        {
			for(str++; *str; str++)
			{
				switch(*str)
				{
				case 'p':
					argn++;
					strcat(open_print_file,argv[argn]); //连接两个字符串，连接后的字符串放在str_temp中
					socket_send(open_print_file);//发送包含文件名的字符串到server
					break;	
				case 'r':
					argn++;
					if(my_strstr(argv[argn],"close",Data_SIZE)!=NULL)
					{
						printf("close\n");
						socket_send("close_print_file");//发送关闭文件指令
					}
					if(my_strstr(argv[argn],"continue",Data_SIZE)!=NULL)
					{
						printf("continue\n");
						socket_send("continue_print_file");//发送关闭文件指令
					}
					break;	
					
				case 'j':
					argn++;
					printf("%s\n",argv[argn]);
					//if (SettingRegistry::getInstance()->loadJSONsettings(argv[argn], last_settings_object))
					//{
						//cura::logError("Failed to load json file: %s\n", argv[argn]);
					//	exit(1);
					//}
					break;
				case 's':
					;
					break;
				default:
					print_usage();
					exit(1);
					break;
				}
			}
		}
        else
        {
            print_usage();
            exit(1);
        }
    }
}


int main(int argc, char **argv)  
{  
    if (argc < 2)
    {
        print_usage();
        exit(1);
    }

    else if (stringcasecompare(argv[1], "slice") == 0)
    {
        slice(argc, argv);
    }
	else
	{
		print_usage();
	}
	
	return 0;
} 
