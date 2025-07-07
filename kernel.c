#include <stdint.h>
#include "font8x8_basic.h"

uint64_t frameBufferBase;
uint32_t pixelsPerScanLine;

void plotPixel(int x, int y, int color) {
    *((uint32_t*)(frameBufferBase + 4 * pixelsPerScanLine * y + 4 * x)) = color;
}

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

void printString(const char* str, int x, int y) {
    while (*str) {
        printChar(*str++, x, y);
        x += 8;
    }
}

void kernel_main(uint64_t fbb, uint32_t pps) __attribute__((section(".entry")));
void kernel_main(uint64_t fbb, uint32_t pps) {
    frameBufferBase = fbb;
    pixelsPerScanLine = pps;

    printString("Kernel: Kernel Started", 10, 10);
    printString("Kernel: Nothing to do; HALTING", 10, 18);

    while (1) {
    }
}
