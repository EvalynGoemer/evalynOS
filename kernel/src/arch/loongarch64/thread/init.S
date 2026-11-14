.globl thread_init_trampoline
.type thread_init_trampoline, @function

.equiv crmd, 0x0

thread_init_trampoline:
    move $a0, $s0
    move $a1, $s1
    move $a2, $s2
    move $a3, $s3
    move $a4, $s4
    move $a5, $s5
    move $a6, $s6
    move $a7, $s7
    ld.d $t0, $sp, 0 # entry
    ld.d $ra, $sp, 8 # exit
    addi.d $sp, $sp, 16

    # enable interrupts
    li.d  $t1, (1 << 2)
    li.d  $t2, (1 << 2)
    csrxchg $t1, $t2, crmd
    jr $t0
