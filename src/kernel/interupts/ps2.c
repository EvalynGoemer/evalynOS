#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"

volatile int ps2InteruptsTriggered = 0;
volatile unsigned char ps2LastScanCode = 0;
volatile int ps2IrqFired = 0;

#define LINE_WIDTH 80

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void ps2_isr(__attribute__((unused)) void* frame) {
    ps2InteruptsTriggered++;
    ps2IrqFired = 1;

    unsigned char scancode = inb(0x60);
    ps2LastScanCode = scancode;

    char result[LINE_WIDTH + 1];
    int len = 0;
    const char *prefix = "ps2InteruptsTriggered: ";
    for (const char *c = prefix; *c; ++c) result[len++] = *c;

    int num = ps2InteruptsTriggered;
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
    printString(result, 10, FB_HEIGHT - 40);

    char result2[LINE_WIDTH + 1];
    len = 0;
    const char *prefix2 = "PS/2 Keyboard Event: ";
    for (const char *c = prefix2; *c; ++c) result2[len++] = *c;

    if (scancode == 0) {
        result2[len++] = '0';
    } else {
        char temp2[16];
        int i = 0;
        while (scancode > 0) {
            temp2[i++] = (scancode % 10) + '0';
            scancode /= 10;
        }
        while (i > 0) result2[len++] = temp2[--i];
    }

    while (len < LINE_WIDTH) result2[len++] = ' ';
    result2[len] = '\0';
    printString(result2, 10, FB_HEIGHT - 32);

    outb(0x20,0x20);
}
