#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <filesystem/filesystem.h>

struct file_node* files = NULL;

int register_file(struct file* file) {
    struct file_node* new_file = malloc(sizeof(struct file_node));
    new_file->file = file;
    new_file->next = files;
    files = new_file;

    return 1;
}

int fs_read(char* path, char* return_data, int read_length) {
    struct file_node* current = files;

    while (current != NULL) {
        if (strcmp(current->file->path, path) == 0) {
            return current->file->read(path, return_data, read_length);
        }

        current = current->next;
    }
    return -1;
}

int fs_write(char* path, char* write_data, int write_length) {
    struct file_node* current = files;

    while (current != NULL) {
        if (strcmp(current->file->path, path) == 0) {
            return current->file->write(path, write_data, write_length);
        }

        current = current->next;
    }
    return -1;
}
