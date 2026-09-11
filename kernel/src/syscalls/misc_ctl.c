#include <stddef.h>
#include <stdio.h>
#include <syscalls/syscalls.h>

#define MISC_CTL_DBG_PUTC 0xE9

sysret_t syscall_misc_ctl(size_t arg1, size_t arg2) {
    switch (arg1) {
        case MISC_CTL_DBG_PUTC: {
            printf("%c", (char)arg2);
            return (sysret_t){0, 0};
        }
        default: return (sysret_t){0, -1};
    }
}
