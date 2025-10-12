#pragma once
#include <stdint.h>

extern volatile struct limine_framebuffer *framebuffer;

extern int FB_WIDTH;
extern int FB_HEIGHT;

extern void plotPixel(int x, int y, unsigned int color);

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
extern void printChar(char c, int x, int y);

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
extern void printString(const char* str, int x, int y);

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void drawImage(const uint32_t* image, int width, int height, int x0, int y0, int scaleX, int scaleY);

extern void drawFrame(const unsigned char *rleData, int rleLength, int frameWidth, int frameHeight, int startX, int startY, int scale);
extern void clearScreen(int width, int height);

