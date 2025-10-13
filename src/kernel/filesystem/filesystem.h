struct filesystem {
    char rootPath[64];
    int (*read)(char* path, char* return_data, int read_length);
    int (*write)(char* path, char* write_data, int write_length);
};

struct filesystem_node {
    struct filesystem* fs;
    struct filesystem_node* next;
};

extern struct filesystem_node* filesystems;

extern int register_fs(struct filesystem* fs);

extern int fs_read(char* path, char* return_data, int read_length);
extern int fs_write(char* path, char* write_data, int write_length);
