#include <arch/generic/timer/timer.h>
#include <arch/intrin/cpulocal.h>
#include <arch/intrin/paging.h>
#include <sched/wait.h>
#include <stdint.h>
#include <utils/lib.h>
#include <stddef.h>
#include <syscalls/syscalls.h>
#include <arch/generic/panic.h>
#include <arch/generic/thread/exit.h>

sysret_t dispatch_syscall(size_t sysnum, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5, size_t arg6, interrupt_frame_t* frame) {
    UNUSED(arg4);
    UNUSED(arg5);
    UNUSED(arg6);
    UNUSED(frame);

    switch (sysnum) {
        case SYS_MISC_CTL   : return syscall_misc_ctl(arg1, arg2);
        case SYS_ALLOC_ANON : return syscall_anon_alloc(arg1);
        case SYS_FREE_ANON  : return syscall_anon_free(arg1, arg2);
        case SYS_FUTEX_WAIT : return syscall_futex_wait((uint32_t*)arg1, arg2, arg3);
        case SYS_FUTEX_WAKE : return syscall_futex_wake((uint32_t*)arg1, arg2);
        case SYS_EXIT       : thread_exit();
        case SYS_SLEEP_NS   : {
            if (arg1 == 0) return SYSRET_OKAY;
            sched_wait_on(nullptr, arg1);
            return SYSRET_OKAY;
        }
        case SYS_GET_NS: {
            uint64_t ns = timer_get_ns();
            return (sysret_t){.retval = ns, .error = 0};
        }
        default: return SYSRET_ENOSYS;
    }
}
