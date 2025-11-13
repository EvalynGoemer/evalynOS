#pragma once
#include <stdint.h>

extern volatile struct limine_framebuffer *framebuffer;

extern int FB_WIDTH;
extern int FB_HEIGHT;

extern void plotPixel(int x, int y, unsigned int color);
extern void printChar(char c, int x, int y);
extern void printString(const char* str, int x, int y);

void drawImage(const uint32_t* image, int width, int height, int x0, int y0, int scaleX, int scaleY);
