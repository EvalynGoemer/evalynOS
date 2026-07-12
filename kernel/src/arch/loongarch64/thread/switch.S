.equiv crmd,  0x0

.global arch_thread_switch
.type arch_thread_switch, @function

.extern schedule_finalize

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
