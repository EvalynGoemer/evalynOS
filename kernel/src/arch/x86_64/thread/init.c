#include <string.h>
#include <stdint.h>
#include <arch/generic/thread/exit.h>
#include <stdlib.h>
#include <arch/x86_64/cpu/CRx.h>

extern void thread_init_trampoline(void);

uintptr_t arch_prepare_thread_stack(uintptr_t stack_top, uintptr_t entry, uintptr_t arg1, uintptr_t arg2) {
    uintptr_t sp = stack_top;

    sp -= 8; *(uintptr_t*)sp = (uintptr_t)thread_exit;            // exit point
    sp -= 8; *(uintptr_t*)sp = entry;                             // entry point
    sp -= 8; *(uintptr_t*)sp = (uintptr_t)thread_init_trampoline; // return address
    sp -= 8; *(uintptr_t*)sp = arg1;                              // rbx -> rdi (arg1)
    sp -= 8; *(uintptr_t*)sp = arg2;                              // rbp -> rsi (arg2)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r12 -> rdx (arg3)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r13 -> rcx (arg4)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r14 -> r8  (arg5)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r15 -> r9  (arg6)

    return sp;
}

void* arch_allocate_fpu_area() {
    void* area;
    if (xsave_enabled) {
        area = malloc_aligned(xsave_size, 64);
        memset(area, 0, xsave_size);
    } else {
        area = kmalloc(512);
        memset(area, 0, 512);
    }
    return area;
}

void arch_free_fpu_area(void* fpu_area) {
    if (xsave_enabled)
        free(fpu_area);
    else
        kfree(fpu_area, 512);
}
