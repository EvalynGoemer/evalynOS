#include "utils/lib.h"
#include <stddef.h>
#include <assert.h>
#include <syscalls/syscalls.h>
#include <arch/generic/panic.h>

sysret_t dispatch_syscall(size_t sysnum, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5, size_t arg6, interrupt_frame_t* frame) {
    UNUSED(arg3);
    UNUSED(arg4);
    UNUSED(arg5);
    UNUSED(arg6);
    UNUSED(frame);

    switch (sysnum) {
        case SYS_MISC_CTL   : return syscall_misc_ctl(arg1, arg2);
        case SYS_ALLOC_ANON : assert(!"unimplemented");
        case SYS_FREE_ANON  : assert(!"unimplemented");
        case SYS_EXIT       : assert(!"unimplemented");
        default: return (sysret_t){0, -1};
    }
}
