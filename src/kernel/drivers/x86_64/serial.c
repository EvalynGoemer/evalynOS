#include <drivers/x86_64/serial.h>
#include <drivers/x86_64/pic.h>
#include <drivers/x86_64/ports.h>
#include <filesystem/filesystem.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SERIAL_PORT 0x3F8

volatile bool serial_works = false;
volatile uint8_t serial_buffer_index;
volatile char serial_buffer[256] = {'\0'};

static inline void io_wait() {
    outb(0x80, 0);
}

int serialDeviceRead(__attribute__((unused)) char* path, char* return_data, int read_length) {
    for (int i = 0; i < read_length; i++) {
        return_data[i] = serial_buffer[serial_buffer_index];
        serial_buffer[serial_buffer_index] = '\0';
        serial_buffer_index--;
    }
    return 1;
}

int serialDeviceWrite(__attribute__((unused)) char* path, __attribute__((unused)) char* write_data, __attribute__((unused)) int write_length) {
    write_serial(write_data, write_length);
    return write_length;
}

void setup_serial() {
    outb(SERIAL_PORT + 3, 0x00); // disable DLAB to set interrupt register
    outb(SERIAL_PORT + 1, 0x01); // enable interrupts
    outb(SERIAL_PORT + 3, 0x80); // enable DLAB to change baud rate
    outb(SERIAL_PORT + 0, 0x01); // set divsor to 1 for 115200 baud
    outb(SERIAL_PORT + 1, 0x00); // high byte of previous
    outb(SERIAL_PORT + 3, 0x03); // set 8N1 mode
    outb(SERIAL_PORT + 2, 0x07); // set FIFO with 1 byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // enable the irqs
    outb(SERIAL_PORT + 4, 0x1E); // enable loopback for testing
    outb(SERIAL_PORT + 0, 0x69); // send test byte

    if(inb(SERIAL_PORT + 0) != 0x69) {
        serial_works = false;
        return;
    }

    outb(SERIAL_PORT + 4, 0x0F); // disable loopback
    serial_works = true;

    unmask_irq(4);

    struct file* file = malloc(sizeof(struct file));
    strcpy(file->path, "/dev/term/stty");
    file->read = serialDeviceRead;
    file->write = serialDeviceWrite;
    register_file(file);
}

int serial_received() {
    return inb(SERIAL_PORT + 5) & 1;
}

char read_serial() {
    while (serial_received() == 0);

    return inb(SERIAL_PORT);
}

int is_transmit_empty() {
    return inb(SERIAL_PORT + 5) & 0x20;
}

int write_serial(char *string, int write_length) {
    while (is_transmit_empty() == 0);

    int i = 0;

    while (string[i] != '\0' && (i < write_length)) {
        char c = string[i];

        // qemu serial terminal and maybe others expect CRLF and not LF while kernel uses LF so convert
        if (c == '\n') {
            outb(SERIAL_PORT, '\r');
        }

        outb(SERIAL_PORT, c);
        i++;
    }

    return i;
}
