#include "../libc/string.h"

#include "../kernel.h"
#include "../libc/stdio.h"
#include "../libc/stdlib.h"
#include "../filesystem/filesystem.h"
#include "../renderer/fb_renderer.h"
#include "../memory/memory_debug.h"
#include "../hardware/pit.h"
#include "../panic.h"

#ifdef INCLUDE_DEMOS
    #include "../demos/demos.h"
#endif

char *to_upper(const char *s) {
    static char buf[256];
    char *p = buf;

    if (!s) return NULL;

    while (*s && (size_t)(p - buf) < sizeof(buf) - 1) {
        *p++ = (*s >= 'a' && *s <= 'z') ? *s - ('a' - 'A') : *s;
        s++;
    }
    *p = '\0';
    return buf;
}

void start_shell() {
    char charBuffer[64];

    for (int i = 0; i < 64 - 1; ++i) {
        charBuffer[i] = '\0';
    }

    for (int i = 0; i < 62; ++i) {
        charBuffer[i] = ' ';
    }

    int charBufferIndex = 0;

    while (1) {
        pit_sleep_ms(1);
        char keyPressed[1] = {'\0'};
        fs_read("/dev/ps2/kbd", keyPressed, 1);

        printString("Kernel Shell> ", 10, FB_HEIGHT - 16);

        if (keyPressed[0] == '\b') {  // Backspace
            if (charBufferIndex > 0) {
                charBufferIndex--;
                charBuffer[charBufferIndex] = ' ';
                printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
            }
        } else if (keyPressed[0] == '\n') {  // Enter
            if (strncmp("TEST", to_upper(charBuffer), 4) == 0) {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }
                printf("Test command works\n");

                charBufferIndex = 0;
            } else if (strncmp("CREDITS", to_upper(charBuffer), 7) == 0) {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }

                printString("Press any key to scroll one line", 8, 10 * 25);

                char *data = malloc(16 * 1024);
                fs_read("/credits.txt", data, 16 * 1024);

                char line[82];

                int i = 0;
                int lines_printed = 0;
                while (data[i] != '\0') {
                    int ix = 0;

                    while (ix < 80 && data[i] != '\0' && data[i] != '\n') {
                        line[ix] = data[i];
                        ix++;
                        i++;
                    }

                    line[ix] = '\0';
                    printf("%s", line);
                    lines_printed++;
                    if (data[i] == '\n') {
                        i++;
                    }

                    if (lines_printed > 23) {
                        int wasKeyPressed = 0;
                        while (!wasKeyPressed) {
                            char keyPressed[1];
                            fs_read("/dev/ps2/kbd", keyPressed, 1);
                            if (keyPressed[0] != '\0') {
                                wasKeyPressed = 1;
                            }
                            pit_sleep_ms(1);
                        }
                    }
                }

                free(data);

                printString("                                                  ", 8, 10 * 25);
                printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                charBufferIndex = 0;
            } else if (strncmp("CLEARFB", to_upper(charBuffer), 7) == 0 || strncmp("CLSFB", to_upper(charBuffer), 5) == '\0') {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }
                clearScreen(FB_WIDTH, FB_HEIGHT);
                printf("Screen Cleared\n");
                printString("Kernel Shell> ", 10, FB_HEIGHT - 16);
                charBufferIndex = 0;
            } else if (strncmp("CLEAR", to_upper(charBuffer), 5) == 0 || strncmp("CLS", to_upper(charBuffer), 3) == '\0') {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }
                cleark();
                printf("Cleared Terminal\n");
                printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                charBufferIndex = 0;
            } else if (strncmp("MMAP", to_upper(charBuffer), 4) == 0) {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }
                printString(charBuffer, 10, 8);
                printf("Memory Map Printed\n");
                printMemoryMap();
                printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                charBufferIndex = 0;
            } else if (strncmp("FAULT", to_upper(charBuffer), 5) == 0) {
                volatile uint64_t *fault = (volatile uint64_t *)0xDEADBEEF;
                *fault = 0xDEADBEEF;
            } else if (strncmp("PANIC", to_upper(charBuffer), 5) == 0) {
                panic("You asked for this lmao", 0, 0, 0, 0);
            } else {
                int validCommand = 0;
                #ifdef INCLUDE_DEMOS
                validCommand = handle_demos(to_upper(charBuffer));
                #endif
                if (!validCommand && charBuffer[0] != ' ') {
                    printString(charBuffer, 10, 8);
                    printf("Invalid Shell Command: %s\n", charBuffer);
                    printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
                }

                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }
                charBufferIndex = 0;
            }
        } else {
            char c = keyPressed[0];
            if (c != 0 && charBufferIndex < 62 - 1) {
                charBuffer[charBufferIndex++] = c;
            }
        }

        printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);
    }
    pit_sleep_ms(1);
}
