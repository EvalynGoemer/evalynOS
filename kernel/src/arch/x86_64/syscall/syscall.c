#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/syscall/syscall.h>
#include <arch/x86_64/cpu/interrupts.h>

void setup_syscall() {
    uint64_t efer = rdmsr(MSR_EFER) | 1;
    wrmsr(MSR_EFER, efer);

    uint64_t star = ((uint64_t)(0x18 | 3) << 48) | ((uint64_t)0x08 << 32);
    wrmsr(MSR_STAR, star);

    wrmsr(MSR_LSTAR, (uint64_t)syscall_handler);

    // mask out CF PF AC ZF SF IE DF OF
    wrmsr(MSR_SFMASK, 0xED5);
}
