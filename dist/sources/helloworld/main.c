#include <klib/syscall.h>
#include <klib/misc_ctl.h>

int puts(char* str) {
    while (*str)
        syscall_misc_ctl_putc(*str++);
    return 0;
}

void _start() {
    puts("Hello from userspace!\n");
    syscall0(SYS_EXIT);
}
