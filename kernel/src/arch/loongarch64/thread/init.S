.globl thread_init_trampoline
.type thread_init_trampoline, @function

thread_init_trampoline:
    move $a0, $s0
    move $a1, $s1
    move $a2, $s2
    move $a3, $s3
    move $a4, $s4
    move $a5, $s5
    move $a6, $s6
    move $a7, $s7
    ld.d $t0, $sp, 0
    jirl $zero, $t0, 0
