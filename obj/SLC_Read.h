#ifndef __SLC_READ_
#define __SLC_READ_

extern int file_continue_flag;//继续解析标识
extern int pixel_bits_x,pixel_bits_y;//frambuffer分辨率
extern float dim;//投影图像放大倍数
extern char open_file_flag;//打开关闭文件标识
void OpenSLC(const char file_dir[]);

#endif
