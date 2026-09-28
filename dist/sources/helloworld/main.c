#include <klib/misc_ctl.h>

int puts(char* str) {
    while (*str)
        syscall_misc_ctl_putc(*str++);
    return 0;
}

void _start() {
    while (true) {
        puts("Hello from userspace!\n");
    }
}
