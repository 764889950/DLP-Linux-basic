#include <stdio.h>      //printf()
#include <stdlib.h>     //exit()
#include <stdlib.h>
#include <string.h>
#include <pthread.h> //使用线程功能时要包含该头文件
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>    
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <ctype.h>
#include "DEV_Config.h"
#include "SLC_Read.h"
#include "strstr.h"

#define Data_SIZE 300
char buf_recv[Data_SIZE],buf_send[Data_SIZE]; //套接字发送接收变量 
char open_file_flag=0;//打开关闭文件标识
char print_file_name[300]={0};//打印用的slc文件

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

void *thread_1(void *args)//套接字通信
{
	//套接字Server
    int server_sockfd,client_sockfd;  
    int server_len,client_len;  
    struct sockaddr_in server_address;  
    struct sockaddr_in client_address;  
      
    server_address.sin_family = AF_INET;  
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  
    server_address.sin_port = 8080;  
    server_len = sizeof(server_address);  
      
    server_sockfd = socket(AF_INET,SOCK_STREAM,0);  
      
    bind(server_sockfd,(struct sockaddr *)&server_address,server_len);  
      
    listen(server_sockfd,5);  
    printf("server waiting for connect\n");  
      
    client_len = sizeof(client_address);  
	
	while(1)
	{
		client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,(socklen_t *)&client_len);  
		printf("连接成功\n"); 
		
		if(recv(client_sockfd,buf_recv,Data_SIZE,0) == -1)  
		{  
			perror("recv");  
			exit(EXIT_FAILURE);  
		}  

		//添加对buf_recv的操作
		//-------------------------------------------
		if(my_strstr(buf_recv,"open_print_file:",Data_SIZE)!=NULL)
		{
			strcpy(print_file_name,&buf_recv[strlen("open_print_file:")]);
			open_file_flag=1;//打开打印文件
			printf("open_print_file:%s\n",&buf_recv[strlen("open_print_file:")]); 
		}
		if(my_strstr(buf_recv,"close_print_file",Data_SIZE)!=NULL)
		{
			open_file_flag=0;//关闭打印文件
			printf("close_print_file\n"); 
		}

		//-------------------------------------------
		
		
		// printf("receive from client is %c\n",char_recv);  
		
		printf("receive from client is %s\n",buf_recv); 

		strcpy(buf_send,buf_recv);//给buf_send赋值，准备发送
		if(send(client_sockfd,buf_send,Data_SIZE,0) == -1)  
		{  
			perror("send");  
			exit(EXIT_FAILURE);  
		}
	}

    shutdown(client_sockfd,2);  
    shutdown(server_sockfd,2); 
	return NULL;
}


void *thread_2(void *args)
{
	/*
	这里添加不带阻塞的任务函数
	*/
	while(1)
	{
		getchar();
		file_continue_flag = 1;
	}
	return NULL;
}

void *thread_3(void *args)
{
	/*
	这里添加带阻塞的任务函数
	*/
	while(1)
	{
		if(open_file_flag==1)
		{
			printf("打开文件：%s\n",print_file_name); 
			// OpenSLC("./jcad.slc");
			char str_temp[300]="./";
			strcat(str_temp,print_file_name); //连接两个字符串，连接后的字符串放在str_temp中
			OpenSLC(str_temp);
			open_file_flag=0;
			printf("关闭打印文件\n"); 
		}
	}
	return NULL;
}
//......

int main(void)
{	
    //1.System Initialization
    if(DEV_ModuleInit())
	{
		exit(0);
	}

	int ret=0;
	pthread_t id1,id2,id3;
	
	ret=pthread_create(&id1,NULL,thread_1,NULL);//开启线程	
	if(ret)
	{
		printf("create pthread error!\n");
		return -1; 
	}

	ret=pthread_create(&id2,NULL,thread_2,NULL);//开启线程
	if(ret)
	{
		printf("create pthread error!\n");
		return  -1; 
	}
	
	ret=pthread_create(&id3,NULL,thread_3,NULL);//开启线程
	if(ret)
	{
		printf("create pthread error!\n");
		return  -1; 
	}	
	
	pthread_join(id1,NULL);//等待线程id1执行完毕，这里阻塞。等待该线程执行完毕后，继续执行下面的语句，否则从主程序中退出，意味着该程序结束了，线程也就没有机会执行。
	pthread_join(id2,NULL);//等待线程id2执行完毕，这里阻塞。等待该线程执行完毕后，继续执行下面的语句，否则从主程序中退出，意味着该程序结束了，线程也就没有机会执行。
	pthread_join(id3,NULL);//等待线程id2执行完毕，这里阻塞。等待该线程执行完毕后，继续执行下面的语句，否则从主程序中退出，意味着该程序结束了，线程也就没有机会执行。
	printf("main over!\n");
	return 0;

	//3.System Exit
    DEV_ModuleExit();
    return 0;  
}  
