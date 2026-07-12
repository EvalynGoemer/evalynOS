#include <stdint.h>

extern void thread_init_trampoline(void);

uintptr_t arch_prepare_thread_stack(uintptr_t stack_top, uintptr_t entry, uintptr_t arg) {
    uintptr_t sp = stack_top;

    sp -= 8; *(uintptr_t*)sp = entry;                             // entry point
    sp -= 8; *(uintptr_t*)sp = (uintptr_t)thread_init_trampoline; // return address
    sp -= 8; *(uintptr_t*)sp = arg;                               // rbx -> rdi (arg1)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // rbp -> rsi (arg2)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r12 -> rdx (arg3)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r13 -> rcx (arg4)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r14 -> r8  (arg5)
    sp -= 8; *(uintptr_t*)sp = 0;                                 // r15 -> r9  (arg6)

    return sp;
}
