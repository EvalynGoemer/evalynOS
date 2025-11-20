#include <drivers/x86_64/ports.h>
#include <drivers/x86_64/serial.h>
#include <drivers/keyboard.h>

#define SERIAL_PORT 0x3F8

__attribute__((interrupt))
void serial_isr(__attribute__((unused)) void* frame) {
    while (inb(SERIAL_PORT + 5) & 1) {
        char c = inb(SERIAL_PORT);
        if (c != '\0') {
            serial_buffer[serial_buffer_index] = c;
            serial_buffer_index++;

            keyboard_buffer[keyboard_buffer_index] = c;
            keyboard_buffer_index++;
        }
    }

    outb(0x20,0x20);
}
