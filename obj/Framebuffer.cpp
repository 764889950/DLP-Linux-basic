/*****************************************************************************
* | File        :   PDC.c
* | Author      :   Youhua Lin
* | Function    :   Hardware underlying interface
* | Info        :
*                Framebuffer
*----------------
* |	This version:   V1.0
* | Date        :   2020-7-15
* | Info        :   Basic version
*
******************************************************************************/
//DataProcessThread.cpp : implementation file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <fcntl.h>
#include <unistd.h>		//write read等函数
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <stdio.h>
#include <iostream>  
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "Framebuffer.h"
#include "SLC_Read.h"
#include "Framebuffer.h"

using namespace std;

#define FBDEVICE "/dev/fb0"

pixel_Framebuffer pixel_bits;
static int screensize = 0;
unsigned char *pfb = NULL;

void read_Framebuffer(pixel_Framebuffer &temp)
{
	static int fd;
    int ret;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    fd = open(FBDEVICE, O_RDWR);
    if (fd < 0)
    {
        perror("open");
    }
    // printf("open %s success \n", FBDEVICE);

   
    ret = ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    if (ret < 0)
    {
        perror("ioctl");
    }

    ret = ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    if (ret < 0)
    {
        perror("ioctl");
    }
    
	
	//计算屏幕的总大小（字节）
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;//除8表示每个字节8位，bits_per_pixel单位为位
	
	//printf("屏幕的总大小screensize=%d byte\n",screensize);

	//xres_virtual yres_virtual定义了framebuffer内存中一帧的尺寸。xres_virtual yres_virtual必定大于或者等于xres yres，
	//printf("显示信息->xres：%d\n",vinfo.xres);
	//printf("显示信息->yres：%d\n",vinfo.yres);
	//printf("显示信息->xres_virtual：%d\n",vinfo.xres_virtual);
	//printf("显示信息->yres_virtual：%d\n",vinfo.yres_virtual);
	//printf("显示信息->颜色深度：%d\n",vinfo.bits_per_pixel);

	temp.x=vinfo.xres;
	temp.y=vinfo.yres;
	temp.bits_per_pix=vinfo.bits_per_pixel;
	
	close(fd);
}

//14byte文件头
typedef struct
{
	char cfType[2];//文件类型，"BM"(0x4D42)
	long cfSize;//文件大小（字节）
	long cfReserved;//保留，值为0
	long cfoffBits;//数据区相对于文件头的偏移量（字节）
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))的作用是告诉编译器取消结构在编译过程中的优化对齐
 
//40byte信息头
typedef struct
{
	char ciSize[4];//BITMAPFILEHEADER所占的字节数
	long ciWidth;//宽度
	long ciHeight;//高度
	char ciPlanes[2];//目标设备的位平面数，值为1
	int ciBitCount;//每个像素的位数
	char ciCompress[4];//压缩说明
	char ciSizeImage[4];//用字节表示的图像大小，该数据必须是4的倍数
	char ciXPelsPerMeter[4];//目标设备的水平像素数/米
	char ciYPelsPerMeter[4];//目标设备的垂直像素数/米
	char ciClrUsed[4]; //位图使用调色板的颜色数
	char ciClrImportant[4]; //指定重要的颜色数，当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要
}__attribute__((packed)) BITMAPINFOHEADER;
 
typedef struct
{
	unsigned short blue;
	unsigned short green;
	unsigned short red;
	unsigned short reserved;
}__attribute__((packed)) PIXEL;//颜色模式RGB
 
BITMAPFILEHEADER FileHead;
BITMAPINFOHEADER InfoHead;


//在调用该文件时，先执行Framebuffer_init()，在不需要使用framebuffer时，执行Framebuffer_end()
int show_bmp(const char bmp_path[], unsigned char *pfb, unsigned int width, unsigned int height)
{
	FILE *fp;
	unsigned char pix=0;
	int rc;
	int line_x, line_y;
	long int location = 0;//, BytesPerLine = 0;
	int pix_c=4;
 
	fp = fopen(bmp_path, "rb" );
	
	if (fp == NULL)
	{
		// printf("file is NULL\n");
		return(-1 );
	}
 
	rc = fread(&FileHead, sizeof(BITMAPFILEHEADER),1, fp );
	if (rc != 1)
	{
		// printf("read header error!\n");
		fclose(fp );
		return(-2 );
	}
 
	//检测是否是bmp图像
	if (memcmp(FileHead.cfType, "BM", 2) != 0)
	{
		// printf("it's not a BMP file\n");
		fclose(fp );
		return(-3 );
	}
 
	rc = fread((char *)&InfoHead, sizeof(BITMAPINFOHEADER),1, fp );
	if (rc != 1)
	{
		// printf("read infoheader error!\n");
		fclose(fp );
		return(-4 );
	}
 
	//跳转的数据区
	fseek(fp, FileHead.cfoffBits, SEEK_SET);
	//每行字节数
	// BytesPerLine = InfoHead.ciWidth * InfoHead.ciBitCount / 8;
	
	// 图片信息
	// printf("图片宽度（单位为一个像素）InfoHead.ciWidth：%ld\n",InfoHead.ciWidth);
	// printf("图片的高度:%ld,图片的宽度:%ld\n",InfoHead.ciWidth,InfoHead.ciHeight);
	// printf("图片每个像素的位数InfoHead.ciBitCount：%d\n",InfoHead.ciBitCount);
	// printf("图片每行字节数：%ld\n",BytesPerLine);
	
	line_x = line_y = 0;
	//向framebuffer中写BMP图片
	while(!feof(fp))
	{	
		if(InfoHead.ciBitCount == 32)//假如颜色深度是32（4个字节），像素值依次带入
		{
			rc = fread(&pix, 1, 1, fp);
			// if (rc != sizeof(pix))
				// break;
			if (line_x == InfoHead.ciWidth*4)//Framebuffer为4位字节表示，所以ciWidth乘4
			{
				line_x = 0;
				line_y++;
				if(line_y == InfoHead.ciHeight)
					break;
			}
			location = line_x + (InfoHead.ciHeight - line_y - 1) * width * 4;//Framebuffer为4位字节表示，所以width乘4
			//显示每一个像素
			*(pfb + location)=pix;
			line_x++;
		}
		else
		{
			if(InfoHead.ciBitCount == 8)//假如颜色深度是8（1个字节），R=G=B=pix
			{
				rc = fread(&pix, 1, 1, fp);
				// if (rc != sizeof(pix))
					// break;
				pix_c=4;
				while(pix_c--)
				{
					if (line_x == InfoHead.ciWidth*4)//Framebuffer为4位字节表示，所以ciWidth乘4
					{
						line_x = 0;
						line_y++;
						if(line_y == InfoHead.ciHeight)
							break;
					}
					location = line_x + (InfoHead.ciHeight - line_y - 1) * width * 4;//Framebuffer为4位字节表示，所以width乘4
					//显示每一个像素
					if(pix_c==0)
					{
						*(pfb + location)=255;//透明度
					}
					else
					{
						*(pfb + location)=pix;
					}
					line_x++;
				}
			}
		}
	}
	
	fclose(fp );
	
	// printf("show_bmp function over\n");
	
	return(0);
}


int fb_fd;//Framebuffer文件描述符

void Framebuffer_init()
{
	//Framebuffr初始化并打开
    fb_fd = open(FBDEVICE, O_RDWR);
	read_Framebuffer(pixel_bits);
	pixel_bits_x=pixel_bits.x;
	pixel_bits_y=pixel_bits.y;
	//内存映射
	screensize = pixel_bits.x * pixel_bits.y * pixel_bits.bits_per_pix / 8;//除8表示每个字节8位，bits_per_pixel单位为位
	pfb = (unsigned char *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	
}

void Framebuffer_end()
{
	//删除对象映射
	munmap(pfb, screensize);
	close(fb_fd);				//关闭Frambuffer文件
}
