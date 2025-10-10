#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"
#include "../panic.h"

#define LINE_WIDTH 80
volatile int genericInteruptsTriggered = 0;

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void generic_isr(__attribute__((unused)) void* frame) {
    genericInteruptsTriggered++;

    if(genericInteruptsTriggered > 10) {
        panic("Too many unhandled interupts");
    }

    if (genericInteruptsTriggered % 100 == 0) {
        char result[LINE_WIDTH + 1];
        int len = 0;

        const char *prefix = "genericInteruptsTriggered: ";
        for (const char *c = prefix; *c; ++c) result[len++] = *c;

        int num = genericInteruptsTriggered;
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
    outb(0xa0,0x20);
}
