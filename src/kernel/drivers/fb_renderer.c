#include <stdint.h>

#include <utils/globals.h>
#include <misc//font8x8_basic.h>

volatile struct limine_framebuffer *framebuffer;
int FB_WIDTH;
int FB_HEIGHT;

void plotPixel(int x, int y, uint32_t color) {
    if (x >= FB_WIDTH || y >= FB_HEIGHT) {
        return;
    }
    *((volatile uint32_t*)framebuffer->address + y * (framebuffer->pitch >> 2) + x) = color;
}

void printChar(char c, int x, int y) {
    int index = c;
    for (int row = 0; row < FONT_HEIGHT; row++) {
        unsigned char bits = font8x8_basic[index][row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (bits & (1 << col)) {
                plotPixel(x + col, y + row, 0xFFFFFFFF);
            } else {
                plotPixel(x + col, y + row, 0x00000000);
            }
        }
    }
}

void printString(const char* str, int x, int y) {
    while (*str) {
        printChar(*str++, x, y);
        x += 8;
    }
}

void drawImage(const uint32_t* image, int width, int height, int x0, int y0, int scaleX, int scaleY) {
    if (scaleX <= 0) scaleX = 1;
    if (scaleY <= 0) scaleY = 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color = image[y * width + x];
            for (int dy = 0; dy < scaleY; dy++) {
                for (int dx = 0; dx < scaleX; dx++) {
                    plotPixel(x0 + x * scaleX + dx, y0 + y * scaleY + dy, color);
                }
            }
        }
    }
}
