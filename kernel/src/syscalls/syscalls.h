#pragma once
#include <stddef.h>
#include <arch/generic/panic.h>

#define SYS_MISC_CTL          0
#define SYS_ALLOC_ANON        1
#define SYS_FREE_ANON         2
#define SYS_EXIT              3
#define SYS_SLEEP_NS          4
#define SYS_GET_NS            5
#define SYS_FUTEX_WAIT        6
#define SYS_FUTEX_WAKE        7

typedef struct {
    size_t retval;
    size_t error;
} sysret_t;

#define SYSRET_OKAY      (sysret_t){0,  0}
#define SYSRET_ENOSYS    (sysret_t){0, -1}
#define SYSRET_ENOMEM    (sysret_t){0, -2}
#define SYSRET_EINVAL    (sysret_t){0, -3}
#define SYSRET_EAGAIN    (sysret_t){0, -4}
#define SYSRET_ETIMEDOUT (sysret_t){0, -5}

extern sysret_t dispatch_syscall(size_t sysnum, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5, size_t arg6, interrupt_frame_t* frame);
extern sysret_t syscall_misc_ctl(size_t arg1, size_t arg2);
extern sysret_t syscall_anon_alloc(size_t size);
extern sysret_t syscall_anon_free(size_t base, size_t len);
extern sysret_t syscall_futex_wait(uint32_t* uptr, uint32_t expected, uint64_t timeout);
extern sysret_t syscall_futex_wake(uint32_t* uptr, int count);
