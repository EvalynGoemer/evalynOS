#include <stddef.h>
#include <stdio.h>
#include <syscalls/syscalls.h>

#define MISC_CTL_DBG_PUTC 0xE9

[[gnu::weak]] sysret_t handle_arch_misc_ctl(size_t arg1, size_t arg2) {
    UNUSED(arg1);
    UNUSED(arg2);
    return SYSRET_ENOSYS;
};

sysret_t syscall_misc_ctl(size_t arg1, size_t arg2) {
    switch (arg1) {
        case MISC_CTL_DBG_PUTC: {
            printf("%c", (char)arg2);
            return SYSRET_OKAY;
        }
        default:
            return handle_arch_misc_ctl(arg1, arg2);
    }
}
