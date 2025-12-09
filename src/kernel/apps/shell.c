#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils/panic.h>
#include <drivers/x86_64/pit.h>
#include <filesystem/filesystem.h>
#include <memory/debug.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <scheduler/scheduler.h>
#include <scheduler/switch.h>
#include "shell.h"

void badapple_kthread() {
    uintptr_t start_virtual = 0x4000;
    size_t page_size = 0x1000;
    size_t needed_mem = 16 * 1024 * 1024;

    for (size_t i = start_virtual; i < start_virtual + needed_mem; i += page_size) {
        uintptr_t pa = (uintptr_t)allocate_page();
        vmm_map_page(get_current_thread()->pagemap, i, pa, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
    }

    uintptr_t stack_top = 0x80000000;
    size_t stack_size = 64 * 1024;

    for (size_t i = stack_top; i > stack_top - stack_size; i -= page_size) {
        uintptr_t pa = (uintptr_t)allocate_page();
        vmm_map_page(get_current_thread()->pagemap, i, pa, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
    }

    fs_read("/badapple.bin", (void *)0x4000, needed_mem);

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

void execute_commands(const char *cmd) {
    if (strcmp("TEST", to_upper(cmd)) == 0) {
        printf("Test Command Executed\n");
        return;
    }
    if (strcmp("CREDITS", to_upper(cmd)) == 0) {
        char * data = malloc(16 * 1024);
        fs_read("/credits.txt", data, 16 * 1024);

        bool nextLineMessagePrinted = false;
        int lineCount = 0;
        char *ptr = data;
        char *lineStart = ptr;

        while (*ptr) {
            if (nextLineMessagePrinted) {
                printf("\x1b[2K\r"); // ansi for clear line and return to start of line
                nextLineMessagePrinted = false;
            }

            if (*ptr == '\n') {
                lineCount++;
                *ptr = '\0';
                printf("%s\n", lineStart);
                lineStart = ptr + 1;
            }

            if (lineCount == 20) {
                char keyPressed[1] = {'\0'};

                while (keyPressed[0] == '\0') {
                    if (!nextLineMessagePrinted) {
                        printf("Press any key to continute");
                        nextLineMessagePrinted = true;
                    }
                    pit_sleep_ms(10);
                    fs_read("/dev/kbd", keyPressed, 1);
                }

                lineCount = 0;
            }

            ptr++;
        }

        if (*lineStart) {
            printf("%s\n", lineStart);
        }

        free(data);
        return;
    }
    if (strcmp("MMAP", to_upper(cmd)) == 0) {
        printMemoryMap();
        return;
    }
    if (strcmp("BADAPPLE", to_upper(cmd)) == 0) {
            pagemap_t* pagemap = new_pagemap();
            create_thread(badapple_kthread, pagemap);
            printf("Started playing BAD APPLE in userspace\n");
        return;
    }
    if ((strcmp("CLEAR", to_upper(cmd)) == 0) || (strcmp("CLS", to_upper(cmd)) == 0)) {
        printf("\x1b[2J\x1b[H"); // ansi for clear screen and go home
        return;
    }
    if (strcmp("PANIC", to_upper(cmd)) == 0) {
        panic("You asked for this lmao", NULL);
    }
    if (strcmp("FAULT", to_upper(cmd)) == 0) {
        volatile uint64_t *fault = (volatile uint64_t *)0xDEADBEEF;
        *fault = 0xDEADBEEF;
    }

    if (strcmp("", to_upper(cmd)) != 0) {
        printf("Invalid Command: %s\n", cmd);
    }
}

void start_shell() {
    char typingBuffer[64] = {'\0'};
    int typingBufferIndex = 0;

    bool newLineStarted = true;

    while (1) {
        if (newLineStarted) {
            printf("\x1b[2K\r"); // ansi for clear line and return to start of line
            printf("Kernel Shell> ");
            newLineStarted = false;
        }

        char keyPressed[1] = {'\0'};
        fs_read("/dev/kbd", keyPressed, 1);

        switch (keyPressed[0]) {
            case '\b':
                if ((typingBufferIndex - 1) > -1) {
                    typingBuffer[--typingBufferIndex] = '\0';
                    printf("\b \b");
                }
                break;
            case '\x7F': // serial port sends [BACKSPACE (\b)] as [DEL (\x7F)] so pretend it its [BACKSPACE (\b)]
                if ((typingBufferIndex - 1) > -1) {
                    typingBuffer[--typingBufferIndex] = '\0';
                    printf("\b \b");
                }
                break;
            case '\n':
                printf("\n");
                execute_commands(typingBuffer);

                typingBufferIndex = 0;
                memset(typingBuffer, '\0', 64);
                newLineStarted = true;
                break;
            case '\r': // serial port sends [ENTER (\n)] as [RETURN (\r)] so pretend it its [ENTER (\n)]
                printf("\n");
                execute_commands(typingBuffer);

                typingBufferIndex = 0;
                memset(typingBuffer, '\0', 64);
                newLineStarted = true;
                break;
            default:
                if (keyPressed[0] != '\0') {
                    if ((typingBufferIndex + 1) <= 63) {
                        typingBuffer[typingBufferIndex++] = keyPressed[0];
                        printf("%c", keyPressed[0]);
                    }
                }
                break;
        }
    }
}
