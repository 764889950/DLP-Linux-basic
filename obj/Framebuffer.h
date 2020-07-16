#ifndef __FRAMEBUFFER_
#define __FRAMEBUFFER_

extern unsigned char *pfb;

typedef struct pixel{int x;int y;int bits_per_pix;} pixel_Framebuffer;

void Framebuffer_init();
void Framebuffer_end();
void read_Framebuffer(pixel_Framebuffer &temp);
int show_bmp(const char bmp_path[], unsigned char *pfb, unsigned int width, unsigned int height);

#endif
