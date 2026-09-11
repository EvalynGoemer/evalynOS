.globl arch_thread_switch
.type arch_thread_switch, @function
.globl arch_switch_to_user
.type arch_switch_to_user, @function
.global arch_finalize_user_switch
.type arch_finalize_user_switch, @function

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

# a0 = user entry
# a1 = user stack top
arch_switch_to_user:
    # zero registers skipping zero, sp, a0, a1
    move x1,  zero
    move x3,  zero
    move x4,  zero
    move x5,  zero
    move x6,  zero
    move x7,  zero
    move x8,  zero
    move x9,  zero
    move x12, zero
    move x13, zero
    move x14, zero
    move x15, zero
    move x16, zero
    move x17, zero
    move x18, zero
    move x19, zero
    move x20, zero
    move x21, zero
    move x22, zero
    move x23, zero
    move x24, zero
    move x25, zero
    move x26, zero
    move x27, zero
    move x28, zero
    move x29, zero
    move x30, zero
    move x31, zero

    # disable interrupts
    csrc sstatus, (1 << 1)
    # clear SPP
    li t0, (1 << 8)
    csrc sstatus, t0
    # set SPIE
    li t0, (1 << 5)
    csrs sstatus, t0

    csrw sepc, a0
    mv sp, a1

    move a0, zero
    move a1, zero

    sret

arch_finalize_user_switch:
    ret
