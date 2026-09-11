.global arch_thread_switch
.type arch_thread_switch, @function
.global arch_switch_to_user
.type arch_switch_to_user, @function
.global arch_finalize_user_switch
.type arch_finalize_user_switch, @function

.extern schedule_finalize

.equiv crmd, 0x0
.equiv prmd, 0x1
.equiv era,  0x6


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
    # zero registers skipping zero, sp, a0, a1
    move  $r1,  $zero
    move  $r2,  $zero
    move  $r6,  $zero
    move  $r7,  $zero
    move  $r8,  $zero
    move  $r9,  $zero
    move  $r10, $zero
    move  $r11, $zero
    move  $r12, $zero
    move  $r13, $zero
    move  $r14, $zero
    move  $r15, $zero
    move  $r16, $zero
    move  $r17, $zero
    move  $r18, $zero
    move  $r19, $zero
    move  $r20, $zero
    move  $r21, $zero
    move  $r22, $zero
    move  $r23, $zero
    move  $r24, $zero
    move  $r25, $zero
    move  $r26, $zero
    move  $r27, $zero
    move  $r28, $zero
    move  $r29, $zero
    move  $r30, $zero
    move  $r31, $zero

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
