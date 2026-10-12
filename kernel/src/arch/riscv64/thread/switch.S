.option arch, +d

#include <utils/lib.h>
#define ZEROED_GPRS 1,3,4,5,6,7,8,9,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31

.globl arch_thread_switch
.type arch_thread_switch, @function
.global arch_thread_pivot
.type arch_thread_pivot, @function
.globl arch_switch_to_user
.type arch_switch_to_user, @function
.global arch_fpu_save
.type arch_fpu_save, @function
.global arch_fpu_restore
.type arch_fpu_restore, @function
.global arch_finalize_user_switch
.type arch_finalize_user_switch, @function

.extern schedule_finalize

# a0 = new ip
# a1 = new sp
arch_thread_pivot:
    move sp, a1
    jr a0

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
    ITER_REGS(ZEROED_GPRS, move x\i, zero)

    # enable fpu
    li  t0, (3 << 13)
    csrs sstatus, t0
    fscsr zero

    ITER_REGS(ZERO_TO_31, fmv.d.x f\i, zero)

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

# a0 = fpu save pointer
arch_fpu_save:
    # check if fpu is dirty
    csrr t0, sstatus
    li   t1, 0x6000
    and  t0, t0, t1
    # branch if it is not
    bne  t0, t1, .Lfpu_save_done

    frcsr t0
    sd t0, 256(a0)
    ITER_REGS(ZERO_TO_31, fsd f\i, (\i * 8)(a0))

    # set fpu to clean
    li t0, 0x2000
    csrc sstatus, t0
.Lfpu_save_done:
    ret

# a0 = fpu save pointer
arch_fpu_restore:
    ld t0, 256(a0)
    fscsr t0
    ITER_REGS(ZERO_TO_31, fld f\i, (\i * 8)(a0))

    # set fpu to clean
    li t0, 0x2000
    csrc sstatus, t0
    ret

arch_finalize_user_switch:
    ret
