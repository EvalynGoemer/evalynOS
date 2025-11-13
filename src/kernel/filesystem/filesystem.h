#pragma once

struct file {
    char path[256];
    int (*read)(char* path, char* return_data, int read_length);
    int (*write)(char* path, char* write_data, int write_length);
};

struct file_node {
    struct file* file;
    struct file_node* next;
};

extern struct file_node* files;

extern int register_file(struct file* fs);

extern int fs_read(char* path, char* return_data, int read_length);
extern int fs_write(char* path, char* write_data, int write_length);
