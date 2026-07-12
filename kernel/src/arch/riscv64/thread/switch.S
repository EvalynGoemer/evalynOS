.globl arch_thread_switch
.type arch_thread_switch, @function

.extern schedule_finalize

# a0 = prev thread_t*
# a1 = next thread_t*
arch_thread_switch:
    addi sp, sp, -104
    sd ra,  96(sp)
    sd s0,  88(sp)
    sd s1,  80(sp)
    sd s2,  72(sp)
    sd s3,  64(sp)
    sd s4,  56(sp)
    sd s5,  48(sp)
    sd s6,  40(sp)
    sd s7,  32(sp)
    sd s8,  24(sp)
    sd s9,  16(sp)
    sd s10,  8(sp)
    sd s11,  0(sp)

    # swap stacks
    sd sp, 0(a0)
    ld sp, 0(a1)

    # finish the switch on new stack
    call schedule_finalize

    # unlock the scheduler spinlock
    fence rw, w
    sw zero, 56(tp)

    # enable interrupts
    csrsi sstatus, 2

    ld s11,  0(sp)
    ld s10,  8(sp)
    ld s9,  16(sp)
    ld s8,  24(sp)
    ld s7,  32(sp)
    ld s6,  40(sp)
    ld s5,  48(sp)
    ld s4,  56(sp)
    ld s3,  64(sp)
    ld s2,  72(sp)
    ld s1,  80(sp)
    ld s0,  88(sp)
    ld ra,  96(sp)
    addi sp, sp, 104
    ret
