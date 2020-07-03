#include <stdio.h>      //printf()
#include <stdlib.h>     //exit()
#include "DEV_Config.h"
#include "parameter.h"
#include "SLC_Read.h"


int main(void)
{
    //1.System Initialization
    if(DEV_ModuleInit())
        exit(0);
  

	slc_main();//SLC文件读取初始化
	
	while(1);

    //3.System Exit
    DEV_ModuleExit();
    return 0;
}
