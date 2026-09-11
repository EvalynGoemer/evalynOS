#pragma once
#include <stddef.h>
#include <arch/generic/panic.h>

#define SYS_MISC_CTL          0
#define SYS_ALLOC_ANON        1
#define SYS_FREE_ANON         2
#define SYS_EXIT              3

typedef struct {
    size_t retval;
    size_t error;
} sysret_t;

extern sysret_t dispatch_syscall(size_t sysnum, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5, size_t arg6, interrupt_frame_t* frame);
extern sysret_t syscall_misc_ctl(size_t arg1, size_t arg2);
