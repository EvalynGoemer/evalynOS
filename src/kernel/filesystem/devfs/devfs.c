#include "devfs.h"
#include "../filesystem.h"
#include "../../libc/string.h"
#include "../../libc/stdlib.h"

struct device_node* devices = NULL;

int devfsRead(char* path, char* return_data, int read_length) {
    struct device_node* current = devices;
    struct device_node* best_device_match = NULL;
    int best_match_len = 0;

    while (current != NULL) {
        int match_len = 0;
        while (current->device->fullPath[match_len] && path[match_len] && current->device->fullPath[match_len] == path[match_len])
            match_len++;

        if (current->device->fullPath[match_len] == '\0' && (path[match_len] == '/' || path[match_len] == '\0') &&
            match_len > best_match_len) {
            best_match_len = match_len;
        best_device_match = current;
            }

            current = current->next;
    }

    if (best_device_match != NULL) {
        return best_device_match->device->read(path, return_data, read_length);
    } else {
        return -2;
    }
}

int devfsWrite(char* path, char* write_data, int write_length) {
    struct device_node* current = devices;
    struct device_node* best_device_match = NULL;
    int best_match_len = 0;

    while (current != NULL) {
        int match_len = 0;
        while (current->device->fullPath[match_len] && path[match_len] && current->device->fullPath[match_len] == path[match_len])
            match_len++;

        if (current->device->fullPath[match_len] == '\0' && (path[match_len] == '/' || path[match_len] == '\0') &&
            match_len > best_match_len) {
            best_match_len = match_len;
        best_device_match = current;
            }

            current = current->next;
    }

    if (best_device_match != NULL) {
        return best_device_match->device->write(path, write_data, write_length);
    } else {
        return -2;
    }
}

void init_devfs() {
    struct filesystem* devfs = malloc(sizeof(struct filesystem));
    strcpy(devfs->rootPath, "/dev");
    devfs->read = devfsRead;
    devfs->write = devfsWrite;

    register_fs(devfs);
}

int register_device(struct device* device) {
    struct device_node* new_device = malloc(sizeof(struct device_node));
    new_device->device = device;
    new_device->next = devices;
    devices = new_device;

    return 1;
};
