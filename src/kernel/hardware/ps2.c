#include "ports.h"
#include "pic.h"
#include "pit.h"

#include "../filesystem/devfs/devfs.h"
#include "../memory/pmm.h"
#include "../libc/string.h"
#include "../libc/stdlib.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_STATUS_INPUT_BUFFER_FULL 0x02
#define PS2_STATUS_OUTPUT_BUFFER_FULL 0x01

volatile uint8_t kbd_buffer_index;
volatile char kbd_buffer[256] = {'\0'};

static inline void io_wait() {
    outb(0x80, 0);
}

int kbdDeviceRead(__attribute__((unused)) char* path, char* return_data, int read_length) {
    for (int i = 0; i < read_length; i++) {
        return_data[i] = kbd_buffer[kbd_buffer_index];
        kbd_buffer[kbd_buffer_index] = '\0';
        kbd_buffer_index--;
    }
    return 1;
}

int kbdDeviceWrite(__attribute__((unused)) char* path, __attribute__((unused)) char* write_data, __attribute__((unused)) int write_length) {
    return -1;
}

// Taken and cleaned up and ported from "init_keyboard()" from https://codeberg.org/NerdNextDoor/arikoto/src/commit/ad2620feab9658b2b28a5abc346a8d7fc565bd9b/kernel/src/misc/keyboard.c
// Copyright (c) 2025 NerdNextDoor All rights reserved.
// NCSA/University of Illinois Open Source License: https://codeberg.org/NerdNextDoor/arikoto/src/branch/master/LICENSE.md
void setup_ps2() {
    outb(PS2_COMMAND_PORT, 0xAD);
    io_wait();
    outb(PS2_COMMAND_PORT, 0xA7);
    io_wait();

    inb(PS2_DATA_PORT);
    io_wait();

    outb(PS2_COMMAND_PORT, 0x20);
    io_wait();

    uint8_t status = inb(PS2_DATA_PORT);
    io_wait();
    status |=1;
    status &= ~(1 << 1);

    outb(PS2_COMMAND_PORT, 0x60);
    io_wait();
    outb(PS2_DATA_PORT, status);
    io_wait();

    outb(PS2_COMMAND_PORT, 0xAE);
    io_wait();

    outb(PS2_DATA_PORT, 0xFF);
    io_wait();

    int timeout = 1000;
    uint8_t response;
    while (timeout--) {
        if ((inb(PS2_STATUS_PORT) & 1) != 0) {
            response = inb(PS2_DATA_PORT);
            if (response == 0xFA) break;
        }
        pit_sleep_ms(1);
    }

    outb(PS2_DATA_PORT, 0xF0);
    io_wait();
    outb(PS2_DATA_PORT, 0x02);
    io_wait();

    while ((inb(PS2_STATUS_PORT) & 1) != 0) {
        inb(PS2_DATA_PORT);
    }

    unmask_irq(1);

    struct device* kbd = malloc(sizeof(struct device));
    strcpy(kbd->fullPath, "/dev/ps2/kbd");
    kbd->read = kbdDeviceRead;
    kbd->write = kbdDeviceWrite;
    register_device(kbd);

    __asm__ __volatile__("sti");
}
