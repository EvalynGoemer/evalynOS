#include "../libc/string.h"

#include "../kernel.h"
#include "../libc/stdio.h"
#include "../libc/stdlib.h"
#include "../filesystem/filesystem.h"
#include "../renderer/fb_renderer.h"
#include "../memory/memory_debug.h"
#include "../memory/vmm.h"
#include "../memory/pmm.h"
#include "../scheduler/scheduler.h"
#include "../scheduler/switch.h"
#include "../hardware/pit.h"
#include "../panic.h"

int create_once = 0;
int run_once = 0;
void badapple_kthread() {
    if(!run_once) {
        uintptr_t start_virtual = 0x4000;
        size_t page_size = 0x1000;
        size_t num_pages = 16 * 1024 * 1024 / page_size;

        for (size_t i = 0; i < num_pages; i++) {
            uintptr_t va = start_virtual + i * page_size;
            uintptr_t pa = (uintptr_t)allocate_page();
            vmm_map_page(kernel_pagemap, va, pa, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
        }

        uintptr_t stack_top = 0x80000000;
        size_t stack_size = 64 * 1024;
        size_t stack_num_pages = stack_size / page_size;

        for (size_t i = 0; i < stack_num_pages; i++) {
            uintptr_t va = stack_top - (i + 1) * page_size;
            uintptr_t pa = (uintptr_t)allocate_page();
            vmm_map_page(kernel_pagemap, va, pa, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
        }

        fs_read("/badapple.bin", (void *)0x4000, 0x7A1200);

        run_once = 1;
    }
    switch_to_user();
}

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
            } else if (strncmp("BADAPPLE", to_upper(charBuffer), 8) == 0) {
                for (int i = 0; i < 62 - 1; ++i) {
                    charBuffer[i] = ' ';
                }

                if (!create_once) {
                    create_thread(badapple_kthread);
                    printf("Kernel: Started playing BAD APPLE in userspace\n");
                    create_once = 1;
                } else {
                    printf("BAD APPLE is already running\n");
                }
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
}
