#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "../kernel.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
#include "font8x8_basic.h"

volatile struct limine_framebuffer *framebuffer;
int FB_WIDTH;
int FB_HEIGHT;

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void plotPixel(size_t x, size_t y, uint32_t color) {
    *((volatile uint32_t*)framebuffer->address + y * (framebuffer->pitch >> 2) + x) = color;
}

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void printChar(char c, int x, int y) {
    int index = c;
    for (int row = 0; row < 8; row++) {
        unsigned char bits = font8x8_basic[index][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                plotPixel(x + col, y + row, 0xFFFFFFFF);
            } else {
                plotPixel(x + col, y + row, 0x00000000);
            }
        }
    }
}

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void printString(const char* str, int x, int y) {
    while (*str) {
        printChar(*str++, x, y);
        x += 8;
    }
}

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
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

static unsigned int prevFrame[120 * 90];
void drawFrame(const unsigned char* rleData, int rleLength, int frameWidth, int frameHeight, int startX, int startY, int scale) {
    int x = 0, y = 0;
    int pos = 0;

    while (pos + 1 < rleLength && y < frameHeight) {
        int count = rleData[pos++];
        unsigned char value = rleData[pos++];
        unsigned int color = value ? 0xFFFFFFFF : 0x00000000;

        for (int i = 0; i < count && y < frameHeight; i++) {
            int idx = y * frameWidth + x;

            if (prevFrame[idx] != color) {
                prevFrame[idx] = color;

                int px = startX + x * scale;
                int py = startY + y * scale;
                int scaledWidth = scale;
                int scaledHeight = scale;

                for (int dy = 0; dy < scaledHeight; dy++) {
                    int rowY = py + dy;
                    for (int i = 0; i < scaledWidth; i++) {
                        plotPixel(px + i, rowY, color);
                    }
                }
            }

            x++;
            if (x >= frameWidth) {
                x = 0;
                y++;
            }
        }
    }
}

void clearScreen(int width, int height) {
    cleark();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            plotPixel(x, y, 0x00000000);
        }
    }

    printString("Kernel Debug Info", 10, FB_HEIGHT - 64);
    printString("genericInteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 56);
    printString("pitInteruptsTriggered: [Logging Stripped For Preformance]", 10, FB_HEIGHT - 48);
    printString("ps2InteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 40);
    printString("PS/2 Keyboard Event: [Not Updated]", 10, FB_HEIGHT - 32);
}
