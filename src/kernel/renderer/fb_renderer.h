#pragma once
#include <stdint.h>

// Reemember to also change in fb_renderer.c
#define FB_WIDTH 1920
#define FB_HEIGHT 1080

extern void plotPixel(int x, int y, unsigned int color);
extern void printChar(char c, int x, int y);

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
extern void printString(const char* str, int x, int y);

extern void drawImage(const uint32_t* image, int width, int height, int x0, int y0);
extern void drawFrame(const unsigned char *rleData, int rleLength, int frameWidth, int frameHeight, int startX, int startY, int scale);
extern void clearScreen(int width, int height);
