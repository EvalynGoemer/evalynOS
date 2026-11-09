#include "utils/lib.h"
#include "utils/limine.h"
#include <stddef.h>
#include <stdio.h>
#include <syscalls/syscalls.h>

#define MISC_CTL_DBG_PUTC 0xE9
#define MISC_CTL_INITRAMFS 0x01

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
        case MISC_CTL_INITRAMFS: {
            SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
            if (initramfs == nullptr) return SYSRET_ENOENT;
            return (sysret_t){.retval = FROM_HHDM(initramfs), .error = initramfs_size};
        }
        default:
            return handle_arch_misc_ctl(arg1, arg2);
    }
}
