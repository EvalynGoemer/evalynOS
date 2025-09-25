#include "ports.h"
#include "pic.h"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_STATUS_INPUT_BUFFER_FULL 0x02
#define PS2_STATUS_OUTPUT_BUFFER_FULL 0x01

static void ps2_wait_input_empty() {
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL);
}

static void ps2_wait_output_full() {
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL));
}

void setup_ps2() {
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAD);
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xA7);

    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL)
        (void)inb(PS2_DATA_PORT);

    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAA);
    ps2_wait_output_full();
    unsigned char result = inb(PS2_DATA_PORT);
    if (result != 0x55) {
        return;
    }
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAE);


    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0x20);
    ps2_wait_output_full();
    unsigned char config = inb(PS2_DATA_PORT);

    config |= 1 << 0;

    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0x60);
    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, config);

    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, 0xF4);

    unsigned char mask;
    mask = inb(PIC1_DATA);
    mask &= ~(1 << 1);
    outb(PIC1_DATA, mask);

    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, 0xF0);
    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, 0x00);
}
