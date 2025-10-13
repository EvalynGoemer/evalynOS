#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"
#include "../hardware/ps2.h"
#include "../libc/stdio.h"
#include <stdbool.h>

volatile int ps2InteruptsTriggered = 0;

volatile int shiftPressed = 0;
static const unsigned char asciiNoShift[256] = { [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',[0x0C]='-',[0x0D]='=',[0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',[0x15]='y',[0x16]='u',[0x17]='i',[0x18]='o',[0x19]='p',[0x1A]='[',[0x1B]=']',[0x1E]='a',[0x1F]='s',[0x20]='d',[0x21]='f',[0x22]='g',[0x23]='h',[0x24]='j',[0x25]='k',[0x26]='l',[0x27]=';',[0x28]='\'',[0x29]='`',[0x2C]='z',[0x2D]='x',[0x2E]='c',[0x2F]='v',[0x30]='b',[0x31]='n',[0x32]='m',[0x33]=',',[0x34]='.',[0x35]='/',[0x39]=' ',[0x1C] = '\n', [0x0E] = '\b'};

static const unsigned char asciiShift[256] = { [0x02]='!',[0x03]='@',[0x04]='#',[0x05]='$',[0x06]='%',[0x07]='^',[0x08]='&',[0x09]='*',[0x0A]='(',[0x0B]=')',[0x0C]='_',[0x0D]='+',[0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',[0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',[0x1A]='{',[0x1B]='}',[0x1E]='A',[0x1F]='S',[0x20]='D',[0x21]='F',[0x22]='G',[0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=':',[0x28]='\"',[0x29]='~',[0x2C]='Z',[0x2D]='X',[0x2E]='C',[0x2F]='V',[0x30]='B',[0x31]='N',[0x32]='M',[0x33]='<',[0x34]='>',[0x35]='?',[0x39]=' ',[0x1C] = '\n', [0x0E] = '\b'};

#define LINE_WIDTH 80

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void ps2_isr(__attribute__((unused)) void* frame) {
    ps2InteruptsTriggered++;

    unsigned char scancode = inb(0x60);

    if (scancode == 0x2A) {
        shiftPressed = 1;
    }
    if (scancode == 0xAA) {
        shiftPressed = 0;
    }

    if (shiftPressed) {
        kbd_buffer[kbd_buffer_index] = asciiShift[scancode];
        kbd_buffer_index++;
    } else {
        kbd_buffer[kbd_buffer_index] = asciiNoShift[scancode];
        kbd_buffer_index++;
    }

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
    snprintf(result2, (LINE_WIDTH + 1), "PS/2 Keyboard Event: %x                                       ", scancode);

    printString(result2, 10, FB_HEIGHT - 32);

    outb(0x20,0x20);
}
