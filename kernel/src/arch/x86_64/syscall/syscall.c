#include "sched/preempt.h"
#include "sched/scheduler.h"
#include <arch/intrin/cpulocal.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/syscall/syscall.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <syscalls/syscalls.h>

void setup_syscall() {
    uint64_t efer = rdmsr(MSR_EFER) | 1;
    wrmsr(MSR_EFER, efer);

    uint64_t star = ((uint64_t)(0x18 | 3) << 48) | ((uint64_t)0x08 << 32);
    wrmsr(MSR_STAR, star);

    wrmsr(MSR_LSTAR, (uint64_t)syscall_handler);

    // mask out CF PF AC ZF SF IE DF OF
    wrmsr(MSR_SFMASK, 0xED5);
}

#define SYS_MISC_CTL_X86_64_SET_FSBASE 0x100

sysret_t handle_arch_misc_ctl(size_t arg1, size_t arg2) {
    switch (arg1) {
        case SYS_MISC_CTL_X86_64_SET_FSBASE:
            thread_t* thread = CPU_LOCAL_GET_CURRENT_THREAD();
            thread->user_tls_addr = arg2;
            wrmsr(MSR_FSBASE, arg2);
            return SYSRET_OKAY;
        default: return SYSRET_ENOSYS;
    }
}
