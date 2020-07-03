/*****************************************************************************
* | File        :   DEV_Config.cpp
* | Author      :   Youhua Lin
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2020-7-3
* | Info        :   Basic version
*
******************************************************************************/
#include <unistd.h>
#include "DEV_Config.h"
#include "Debug.h"  //DEBUG()

/**
 * Module Initialize, use library.
 *
 * Example:
 * if(DEV_ModuleInit())
 *   exit(0);
 */
UBYTE DEV_ModuleInit(void)
{
	//这里添加硬件初始化函数

    return 0;
}

/**
 * Module Exit, close library.
 *
 * Example:
 * DEV_ModuleExit();
 */
void DEV_ModuleExit(void)
{
	//这里添加硬件退出函数
}

/**
 * Millisecond delay.
 *
 * @param xms: time.
 *
 * Example:
 * DEV_Delay_ms(500);//delay 500ms
 */
void DEV_Delay_ms(uint32_t xms)
{
    sleep(xms);
}

/**
 * Microsecond delay.
 *
 * @param xus: time.
 *
 * Example:
 * DEV_Delay_us(500);//delay 500us
 */
void DEV_Delay_us(uint32_t xus)
{
    usleep(xus);
}
