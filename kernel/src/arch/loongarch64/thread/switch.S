#include <utils/lib.h>

#define ZEROED_GPRS 1,2,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31

.global arch_thread_switch
.type arch_thread_switch, @function
.global arch_thread_pivot
.type arch_thread_pivot, @function
.global arch_switch_to_user
.type arch_switch_to_user, @function
.global arch_finalize_user_switch
.type arch_finalize_user_switch, @function

.extern schedule_finalize
.extern init_fpu_state

.equiv crmd, 0x0
.equiv prmd, 0x1
.equiv era,  0x6

# a0 = new ip
# a1 = new sp
arch_thread_pivot:
    move $sp, $a1
    jr $a0

# a0 = prev thread_t*
# a1 = next thread_t*
arch_thread_switch:
    addi.d $sp, $sp, -88
    st.d   $ra, $sp,  80
    st.d   $fp, $sp,  72
    st.d   $s0, $sp,  64
    st.d   $s1, $sp,  56
    st.d   $s2, $sp,  48
    st.d   $s3, $sp,  40
    st.d   $s4, $sp,  32
    st.d   $s5, $sp,  24
    st.d   $s6, $sp,  16
    st.d   $s7, $sp,   8
    st.d   $s8, $sp,   0

    # swap stacks
    st.d $sp, $a0, 0
    ld.d $sp, $a1, 0

    # finish the switch on new stack
    bl schedule_finalize

    # unlock the scheduler spinlock
    dbar 0x12
    st.w $zero, $r21, 56

    # enable interrupts
    li.d  $t0, (1 << 2)
    li.d  $t1, (1 << 2)
    csrxchg $t0, $t1, crmd

    ld.d   $s8, $sp,  0
    ld.d   $s7, $sp,  8
    ld.d   $s6, $sp, 16
    ld.d   $s5, $sp, 24
    ld.d   $s4, $sp, 32
    ld.d   $s3, $sp, 40
    ld.d   $s2, $sp, 48
    ld.d   $s1, $sp, 56
    ld.d   $s0, $sp, 64
    ld.d   $fp, $sp, 72
    ld.d   $ra, $sp, 80
    addi.d $sp, $sp, 88
    jr $ra

# a0 = user entry
# a1 = user stack top
arch_switch_to_user:
    bl init_fpu_state
    ITER_REGS(ZEROED_GPRS, move $r\i, $zero)

    # disable interrupts
    li.d  $t0, 0
    li.d  $t1, (1 << 2)
    csrxchg $t0, $t1, crmd

    # plv = 3 && interrupts on
    li.d    $t0, (3 | (1 << 2))
    csrxchg $t0, $t0, prmd

    # set return address
    csrwr $a0, era
    move  $sp, $a1

    move  $a0, $zero
    move  $a1, $zero

    ertn

# nop on loongarch64
arch_finalize_user_switch:
    jr $ra
