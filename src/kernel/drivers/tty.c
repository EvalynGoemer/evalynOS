#include <string.h>
#include <stdlib.h>

#include <flanterm.h>

#include <utils/globals.h>
#include <drivers/x86_64/serial.h>
#include <filesystem/filesystem.h>

int ttyDeviceRead(__attribute__((unused)) char* path, __attribute__((unused)) char* return_data,  __attribute__((unused)) int read_length) {
    return -1;
}

int ttyDeviceWrite(__attribute__((unused)) char* path, char* write_data, int write_length) {
    flanterm_write(ft_ctx, write_data, write_length);

    if(serial_works) {
        write_serial(write_data, write_length);
    }

    return write_length;
}

void setup_tty() {
    struct file* file = malloc(sizeof(struct file));
    strcpy(file->path, "/dev/term/tty");
    file->read = ttyDeviceRead;
    file->write = ttyDeviceWrite;
    register_file(file);
}
