#include "../libc/string.h"

#include "../kernel.h"
#include "../renderer/fb_renderer.h"
#include "../interupts/ps2.h"

#ifdef INCLUDE_DEMOS
    #include "../demos/demos.h"
#endif

void start_shell() {
    static const unsigned char scancode_set1_ascii[256] = { [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',[0x0C]='-',[0x0D]='=',[0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',[0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',[0x1A]='[',[0x1B]=']',[0x1E]='A',[0x1F]='S',[0x20]='D',[0x21]='F',[0x22]='G',[0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=';',[0x28]='\'',[0x29]='`',[0x2C]='Z',[0x2D]='X',[0x2E]='C',[0x2F]='V',[0x30]='B',[0x31]='N',[0x32]='M',[0x33]=',',[0x34]='.',[0x35]='/',[0x39]=' ' };

    char charBuffer[64];

    for (int i = 0; i < 64 - 1; ++i) {
        charBuffer[i] = '\0';
    }

    for (int i = 0; i < 62; ++i) {
        charBuffer[i] = ' ';
    }

    int charBufferIndex = 0;

    while (1) {
        if (ps2IrqFired) {
            printString("Kernel Shell> ", 10, FB_HEIGHT - 16);
            ps2IrqFired = 0;

            if (ps2LastScanCode == 14) { // Backspace
                if (charBufferIndex > 0) {
                    charBufferIndex--;
                    charBuffer[charBufferIndex] = ' ';
                    printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                }
            } else if(ps2LastScanCode == 28) { // Enter
                if(strncmp("TEST", charBuffer, 4) == 0) {
                    for (int i = 0; i < 62 - 1; ++i) {
                        charBuffer[i] = ' ';
                    }
                    printString(charBuffer, 10, 8);
                    printString("Shell Test Command Ran Successfully", 10, 8);
                    printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                    charBufferIndex = 0;
                } else {
                    int validCommand = 0;
                    #ifdef INCLUDE_DEMOS
                    validCommand = handle_demos(charBuffer);
                    #endif
                    for (int i = 0; i < 62 - 1; ++i) {
                        charBuffer[i] = ' ';
                    }
                    charBufferIndex = 0;

                    if(!validCommand) {
                        printString(charBuffer, 10, 8);
                        printString("Invalid Shell Command", 10, 8);
                        printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                    }
                }
            } else {
                char c = scancode_set1_ascii[ps2LastScanCode];
                if (c != 0 && charBufferIndex < 62 - 1) {
                    charBuffer[charBufferIndex++] = c;
                }
            }

            printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
        }

        __asm__ __volatile__("hlt");
    }
}
