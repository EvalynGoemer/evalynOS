.globl thread_init_trampoline
.type thread_init_trampoline, @function

thread_init_trampoline:
    mv a0, s0
    mv a1, s1
    mv a2, s2
    mv a3, s3
    mv a4, s4
    mv a5, s5
    mv a6, s6
    mv a7, s7
    ld t0, 0(sp)
    jalr zero, t0, 0
