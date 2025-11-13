#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <utils/globals.h>
#include <filesystem/filesystem.h>
#include <filesystem/tarfs/tarfs.h>

// Code adapted from https://wiki.osdev.org/USTAR

unsigned char *archive = NULL;

int oct2bin(unsigned char *str, int size) {
    int n = 0;
    unsigned char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

int tarfsRead(char* path, char* return_data, int read_length) {
    char tarPath[256];
    tarPath[0] = '.';
    strcpy(tarPath + 1, path);

    unsigned char *ptr = archive;
    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        if (!memcmp(ptr, tarPath, strlen(tarPath) + 1)) {
            unsigned char *file_data = ptr + 512;
            int bytes_to_copy = (filesize < read_length) ? filesize : read_length;

            for (int i = 0; i < bytes_to_copy; i++) {
                return_data[i] = file_data[i];
            }
            return bytes_to_copy;
        }
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
    return -1;
}


int tarfsWrite(__attribute__ ((unused)) char* path, __attribute__ ((unused)) char* write_data, __attribute__ ((unused))  int write_length) {
    return -1;
}

int init_tarfs() {
    if (module_request.response && module_request.response->module_count > 0) {
        for (uint64_t i = 0; i < module_request.response->module_count; i++ ) {
            if (strcmp(module_request.response->modules[i]->path, "/initramfs.tar") == 0) {
                archive = module_request.response->modules[i]->address;
            }
        }
    }

    if(archive == NULL) {
        return -1;
    }

    unsigned char *ptr = archive;

    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        char *path = (char*)ptr;

        if (filesize > 0) {
            struct file* file = malloc(sizeof(struct file));
            strcpy(file->path, path + 1);
            file->read = tarfsRead;
            file->write = tarfsWrite;
            register_file(file);
        }

        ptr += (((filesize + 511) / 512) + 1) * 512;
    }


    return 0;
}
