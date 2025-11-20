#include <filesystem/filesystem.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

volatile uint8_t keyboard_buffer_index;
volatile char keyboard_buffer[256] = {'\0'};

int kbdDeviceRead(__attribute__((unused)) char* path, char* return_data, int read_length) {
    for (int i = 0; i < read_length; i++) {
        return_data[i] = keyboard_buffer[keyboard_buffer_index];
        keyboard_buffer[keyboard_buffer_index] = '\0';
        keyboard_buffer_index--;
    }
    return 1;
}

int kbdDeviceWrite(__attribute__((unused)) char* path, __attribute__((unused)) char* write_data, __attribute__((unused)) int write_length) {
    return -1;
}

void setup_keyboard() {
    struct file* file = malloc(sizeof(struct file));
    strcpy(file->path, "/dev/kbd");
    file->read = kbdDeviceRead;
    file->write = kbdDeviceWrite;
    register_file(file);
}
