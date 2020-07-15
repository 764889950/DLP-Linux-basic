/*****************************************************************************
* | File        :   SLC_Read.cpp
* | Author      :   Youhua Lin
* | Function    :   Hardware underlying interface
* | Info        :
*                SLC文件解析
*----------------
* |	This version:   V1.0
* | Date        :   2020-7-3
* | Info        :   Basic version
*
******************************************************************************/

//OpenCV的轮廓查找和填充  https://blog.csdn.net/garfielder007/article/details/50866101
//opencv findContours和drawContours使用方法  https://blog.csdn.net/ecnu18918079120/article/details/78428002
//OpenCV关于容器的介绍  https://blog.csdn.net/Ahuuua/article/details/80593388

//opencv
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp" 

//other
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <pthread.h> //使用线程功能时要包含该头文件
#include <unistd.h>	 //write read等函数
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "SLC_Read.h"


using namespace cv;
using namespace std;

int file_continue_flag=0;//继续解析标识
int pixel_bits_x=1920,pixel_bits_y=1080;//frambuffer分辨率
float dim=20;//投影图像放大倍数

//字符数组中查找字符串或字符数组
//pSrc:原字符
//srcSize:原字符长度
//pDest：比对的字符
//dstSize:比对的字符的长度
int FindString(char pSrc[], int srcSize, char pDest[], int dstSize)
{
    int iFind = -1;
    for(size_t i=0;i<(size_t)srcSize;i++){
        int iCnt = 0;
        for (size_t j=0; j<(size_t)dstSize; j++) {
            if(pDest[j] == pSrc[i+j])
                iCnt++;
        }
        if (iCnt==dstSize) {
            iFind = i;
            break;
        }
    }
    return iFind;//返回比对的字符起始位置
}


void OpenSLC(const char file_dir[])
{
	int dx=0,dy=0;	
	FILE *fd_temp;  //将调试数据保存到文本区

	//保存调试数据到文件夹
	fd_temp=fopen("./temp.txt", "w+");
	
	/************处理文件头的字符变量，解析完毕后，关闭当前文件*********************/
	//关于文件的文件头变量
	static char str_temp[300];
	static char str_parameter[300];
	
	float minx, maxx, miny, maxy, minz, maxz;

	FILE * fid = fopen(file_dir,"r");//用于处理文件头的信息
	if(fid == NULL)
    {
        return;
    }
	
	/*此处添加文件头处理,进行信息提取*/
	fread(str_temp,sizeof(char),300,fid);
	//-EXTENTS <minx,maxx miny,maxy minz,maxz> CAD模型 x,y,z轴的范围
	int data_adr = FindString(str_temp,300,(char *)"-EXTENTS",8);
	strcpy(str_parameter,str_temp+data_adr);//提取出XYZ的范围数据
	
	//提取XYZ的范围（前6个浮点值）
	char str[50];//无关变量
	sscanf(str_parameter, "%[(A-Z)|^-]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(A-Z)|^*]", str, &minx, str, &maxx, str, &miny, str, &maxy, str, &minz, str, &maxz, str);
	printf("minx=%.3f,maxx=%.3f,miny=%.3f,maxy=%.3f,minz=%.3f,maxz%.3f\r\n", minx,maxx,miny,maxy,minz,maxz);

	str_parameter[FindString(str_parameter,strlen(str_parameter),(char *)"\r\n",2)]='\0';
	// printf("%s\r\n", str_parameter);
	fprintf(fd_temp,"%s\r\n",str_parameter);
	fclose(fid);
	/*-----------end--------------------------------------------------------------*/
	
	int fd; 		//文件描述符
	char m_data;	//读取到的数据
	float dir = 0;    //d > 0 从上往下看是逆时针
	// int size=0;		//读取到的数据长度
	
	fd = open(file_dir, O_RDONLY);
	//////////////////////////////
	
	/*
	// O_CREAT 若欲打开的文件不存在则自动建立该文件。
	// O_RDONLY 以只读方式打开文件
	// O_WRONLY 以只写方式打开文件
	*/
	
	//【1】CV_8UC1---则可以创建----8位无符号的单通道---灰度图片------grayImg
	//【2】CV_8UC3---则可以创建----8位无符号的三通道---RGB彩色图像---colorImg 
	//【3】CV_8UC4--则可以创建-----8位无符号的四通道---带透明色的RGB图像 
	Mat dst = Mat::zeros(1080, 1920, CV_8UC1);//生成的图片，其分辨率由实际的FrameBuffer来决定
	CvScalar color=cvScalar(0);
	
	vector<Point> contour;       	 //单个轮廓坐标值
	vector<vector<Point>> v_contour; //当前层所有轮廓集合
	
	
	vector<int> flag_swap_vector;	//轮廓排序用
	float flag_swap=0;				//轮廓排序用
	
	

	unsigned int i=0;
	unsigned int j=0;

	unsigned int n_boundary,n_vertices,n_gaps;
	float   n_float,n_layer;

	float   n_polylineX,n_polylineY;
	
    /**************************处理头文件部分**************************/
	while(1)
	{
		i++;
		if(i==2048)
		{
			printf("file error\r\n");
			
			fprintf(fd_temp,"file error\r\n");
			close(fd);
			//关闭调试输出的数据文件
			fclose(fd_temp);
			//////////////////////
			return;
		}

		read(fd, &m_data, 1);
		// printf("m_data=%x\r\n", m_data);

		switch(m_data)
		{
			case 0x0d:
					j=1;
			break;
			case 0x0a:
				if(j==1)
					j=2;
			break;
			case 0x1a:
				if(j==2)
					j=3;
			break;
			default:
				j=0;
			break;
		}
		if(j==3)
			break;
	}
	
	// printf("size=%d\r\n", size);
	/******************************************************************/
	/***************************处理预留部分***************************/
	for(i=0;i<256;i++)
	{
		read(fd, &m_data, 1);
		// fprintf(fd_temp,"m_data=%x\r\n",m_data);
	}
	/******************************************************************/
	/**************************处理样本表部分**************************/
	read(fd, &m_data, 1);
	printf("Sampling Table Size=%x\r\n", m_data);
	// fprintf(fd_temp,"Sampling Table Size=%x\r\n",m_data);
	while(m_data)
	{
		read(fd, &n_float, 4);//Minimum Z Level
		read(fd, &n_float, 4);//Layer Thickness
		// printf("Layer Thickness=%.5f\r\n",n_float);
		fprintf(fd_temp,"Layer Thickness=%.5f\r\n",n_float);
		
		// m_parameter->n_HLayer=n_float;
		//n_totalLayers=(int)((zmax-zmin)/n_float);    //计算出来的总层数
		read(fd, &n_float, 4);    //Line Width Compensation
		read(fd, &n_float, 4);    //Reserved
		m_data--;
	}
	
	long layer=0;//当前正在解析的层
	
	dx=pixel_bits_x/2-(minx+maxx)/2;
	dy=pixel_bits_y/2-(miny+maxy)/2;
	// /*************************处理轮廓数据部分*************************/
	while(1)
	{
		layer++;
		
		fprintf(fd_temp,"第%ld层\r\n",layer);

		read(fd, &n_layer, 4);
		fprintf(fd_temp,"Z轴高度=%.5f\r\n",n_layer);
		
		read(fd, &n_boundary, 4);
		fprintf(fd_temp,"轮廓数=%d\r\n",n_boundary);
		if(n_boundary==0xFFFFFFFF)  //结束符
			break;
		
		for(i=0;i<n_boundary;i++)   //把同一层多个轮廓都放在同一容器中，
		{                           //显示跟数据处理时 要根据起始点和同轮廓的终点相等来判断是否为同一轮廓  
			read(fd, &n_vertices, 4);//一个轮廓环中的点数

			fprintf(fd_temp,"第%d个轮廓环中的点数=%d\r\n",i+1,n_vertices);

			read(fd, &n_gaps, 4);

			contour.clear();//删除容器中的所有元素
			for(j=0;j<n_vertices;j++)
			{
				read(fd, &n_polylineX, 4);
				fprintf(fd_temp,"{%.0f,",n_polylineX*dim+dx); //偏移后的坐标放大，保存调试数据到文件
				read(fd, &n_polylineY, 4);
				fprintf(fd_temp,"%.0f}\r\n",n_polylineY*dim+dy); //偏移后的坐标放大，保存调试数据到文件
				contour.push_back(Point((long)(n_polylineX*dim+dx),(long)(n_polylineY*dim+dy))); //向轮廓坐标尾部添加点坐标
			}

			v_contour.push_back(contour);//追加当前轮廓数据到当前层容器变量中
			contour.clear();//删除容器中的所有元素		  
		}
		

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//通过冒泡法实现容器中轮廓的排序，使得较小轮廓始终位于较大轮廓后，能够判断是否出现交叉异常(注：两个分离的轮廓也会进行排序，不影响填充）
		int n=v_contour.size();//获取轮廓的个数

		for(size_t cmpnum = n-1; cmpnum != 0; --cmpnum)
		{
			for (size_t i = 0; i != cmpnum; ++i)
			{	
				for(size_t k=0;k<v_contour[i+1].size();k++)
				{
	
					flag_swap=pointPolygonTest(v_contour[i], v_contour[i+1][k], false); // 对于每个点都去检测 
					flag_swap_vector.push_back(flag_swap);
				}

				for(size_t z=0;z<flag_swap_vector.size()-1;z++)
				{
					if(flag_swap_vector[z]!=flag_swap_vector[z+1])
					{
						//printf("有存在交叉现象\r\n");
						//这里应该去做相应的异常处理,也可不做处理
					}
				}
				
				flag_swap_vector.clear();//删除容器中的所有元素
				
				if (flag_swap == -1)
				{
					swap(v_contour[i],v_contour[i+1]);
				}
			}
		}	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			
		//清除图像
		dst.setTo(Scalar(0));//把像素点值清零
		
		for(i=0;i<n_boundary;i++)   //把同一层多个轮廓都放在同一容器中，
		{                           //显示跟数据处理时 要根据起始点和同轮廓的终点相等来判断是否为同一轮廓  
			dir = 0;
			for (size_t j = 0; j < v_contour[i].size()-1; j++)
			{
				dir += -0.5*(v_contour[i][j+1].y+v_contour[i][j].y)*(v_contour[i][j+1].x-v_contour[i][j].x);
			}
		
			// a) 存放单通道图像中像素：cvScalar(255);
			// b) 存放三通道图像中像素：cvScalar(255,255,255);
			if(dir > 0)
			{
				//cout << "逆时针：counterclockwise"<< endl;
				fprintf(fd_temp,"逆时\r\n\r\n");
				//填充白色
				color=cvScalar(255);
			}
			else
			{
				//cout << "顺时针：clockwise" << endl;
				fprintf(fd_temp,"顺时\r\n\r\n");
				//填充黑色
				color=cvScalar(0);
			}	
			drawContours( dst,v_contour ,i, color, CV_FILLED );		
		}
		while(!file_continue_flag)
		{
			if(open_file_flag==0)
			{
				fclose(fd_temp);
				close(fd);
				return;
			}
		}
		file_continue_flag = 0;    // 使下次处于一个阻态

		imwrite("./dst.bmp",dst);
		
		v_contour.clear();//删除容器中的所有元素，这里的元素是同一层中所有轮廓数据


		printf("第%ld层\r\n",layer);
		printf("BMP_OK\r\n");	
	}
	
	fclose(fd_temp);
	close(fd);
}



/* // 顺逆时针判断（从上往下） https://blog.csdn.net/qq_37602930/article/details/80496498  */
/*	

vector<Point> contour;       	 //单个轮廓坐标值
vector<vector<Point>> v_contour; //当前层所有轮廓集合
	
contour.clear();//删除容器中的所有元素
contour = {{300,760},{292,748},{291,747},{288,747},{279,746},{269,758},{268,758},{261,745},{260,745},
{260,744},{251,742},{249,742},{238,754},{237,754},{230,739},{229,739},{229,739},{229,738},{230,737},{242,724},
{242,724},{249,737},{263,737},{271,728},{277,737},{295,736},{300,729},{305,736},{323,737},{329,728},{337,737},
{351,737},{358,724},{358,724},{371,737},{371,738},{371,739},{371,739},{370,739},{363,754},{362,754},{351,742},
{349,742},{340,744},{340,745},{339,745},{332,758},{331,758},{321,746},{312,747},{309,747},{308,748},{300,760}};
v_contour.push_back(contour);

contour.clear();//删除容器中的所有元素
contour.push_back(Point(300,760)); //向轮廓坐标尾部添加点坐标
v_contour.push_back(contour);

printf("v_contour.size()=%d\r\n",v_contour.size());//查看容器的大小


contour.clear();//删除容器中的所有元素
// contour.push_back(Point(300,760)); //向轮廓坐标尾部添加点坐标
contour = {{300,760},{292,748},{291,747},{288,747},{279,746},{269,758},{268,758},{261,745},{260,745},
{260,744},{251,742},{249,742},{238,754},{237,754},{230,739},{229,739},{229,739},{229,738},{230,737},{242,724},
{242,724},{249,737},{263,737},{271,728},{277,737},{295,736},{300,729},{305,736},{323,737},{329,728},{337,737},
{351,737},{358,724},{358,724},{371,737},{371,738},{371,739},{371,739},{370,739},{363,754},{362,754},{351,742},
{349,742},{340,744},{340,745},{339,745},{332,758},{331,758},{321,746},{312,747},{309,747},{308,748},{300,760}};
v_contour.push_back(contour);
printf("contour.size()=%d\r\n",contour.size());//查看容器的大小
printf("v_contour.size()=%d\r\n",v_contour.size());
contour.clear();//删除容器中的所有元素

contour.clear();//删除容器中的所有元素
// contour.push_back(Point(300,760)); //向轮廓坐标尾部添加点坐标
contour = {{205,748},{199,731},{199,730},{213,718},{213,718},{217,729},{220,735},{219,736},{206,748},{205,748}};
v_contour.push_back(contour);
printf("contour.size()=%d\r\n",contour.size());//查看容器的大小
printf("v_contour.size()=%d\r\n",v_contour.size());
contour.clear();//删除容器中的所有元素

contour.clear();//删除容器中的所有元素
// contour.push_back(Point(300,760)); //向轮廓坐标尾部添加点坐标
contour = {{394,748},{381,736},{380,735},{383,728},{387,718},{389,720},{401,730},{396,743},{395,748},{394,748}};
v_contour.push_back(contour);
printf("contour.size()=%d\r\n",contour.size());//查看容器的大小
printf("v_contour.size()=%d\r\n",v_contour.size());
contour.clear();//删除容器中的所有元素


// contour.push_back(Point(300,760)); //向轮廓坐标尾部添加点坐标
// contour.clear();//删除容器中的所有元素
// contour.pop_back();//删除容器中的最后一个元素
// v_contour.insert(v_contour.begin(),contour);//在当前轮廓组的首个轮廓前插入一个轮廓
	
/ color=cvScalar(rand()&255, rand()&255, rand()&255 );
*/


	




