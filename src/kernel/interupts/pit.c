#include <stddef.h>

#include "../libc/stdio.h"
#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"

volatile int pitInteruptsTriggered = 0;
#define LINE_WIDTH 80

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void pit_isr(__attribute__((unused)) void* frame) {
    pitInteruptsTriggered++;

    if (term_updated == 1) {
        for (int y = 0; y < TERM_HEIGHT; y++) {
            for (int x = 0; x < TERM_WIDTH; x++) {
                char c = terminal[y][x];
                    printChar(c, 8 + x * 8, 8 + y * 10);
            }
        }
        term_updated = 0;
    }

    if (pitInteruptsTriggered % 100 == 0) {
        char result[LINE_WIDTH + 1];
        int len = 0;

        const char *prefix = "pitInteruptsTriggered: ";
        for (const char *c = prefix; *c; ++c) result[len++] = *c;

        int num = pitInteruptsTriggered;
        if (num == 0) {
            result[len++] = '0';
        } else {
            char temp[16];
            int i = 0;
            while (num > 0) {
                temp[i++] = (num % 10) + '0';
                num /= 10;
            }
            while (i > 0) result[len++] = temp[--i];
        }

        while (len < LINE_WIDTH) result[len++] = ' ';
        result[len] = '\0';

        printString(result, 10, FB_HEIGHT - 48);
    }

    outb(0x20,0x20);
}
