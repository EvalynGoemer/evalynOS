struct device {
    char fullPath[256];
    int (*read)(char* path, char* return_data, int read_length);
    int (*write)(char* path, char* write_data, int write_length);
};

struct device_node {
    struct device* device;
    struct device_node* next;
};

extern struct device_node* devices;

extern void init_devfs();
extern int register_device(struct device* device);
