#include <stdint.h>

extern void thread_init_trampoline(void);

uintptr_t arch_prepare_thread_stack(uintptr_t stack_top, uintptr_t entry, uintptr_t arg) {
    uintptr_t sp = stack_top;

    sp -= 8; *(uintptr_t*)sp = entry;                             // entry point
    sp -= 8; *(uintptr_t*)sp = (uintptr_t)thread_init_trampoline; // ra
    sp -= 8; *(uintptr_t*)sp = 0;                                 // fp
    sp -= 8; *(uintptr_t*)sp = arg;                               // s0 -> a0
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s1 -> a1
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s2 -> a2
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s3 -> a3
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s4 -> a4
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s5 -> a5
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s6 -> a6
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s7 -> a7
    sp -= 8; *(uintptr_t*)sp = 0;                                 // s8 -> a8

    return sp;
}
