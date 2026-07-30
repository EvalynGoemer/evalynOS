.extern dispatch_interrupt
.global interrupt_handler_asm

.equiv crmd,  0x0
.equiv prmd,  0x1
.equiv estat, 0x5
.equiv era,   0x6
.equiv badv,  0x7
.equiv badi,  0x8
.equiv save0, 0x30

.balign 4096
interrupt_handler_asm:
    # cpulocal + 0  = current thread struct
    # cpulocal + 8  = scratch t1
    # cpulocal + 16 = scratch t2
    # cpulocal + 24 = scratch sp
    # current thread + 0 = ksp

    csrwr $t0, save0    # swap save0 and t0 to get cpu local
    st.d  $t1, $t0,  8  # save t1 into cpu local
    st.d  $t2, $t0, 16  # save t2 into cpu local
    move  $t2, $t0      # copy t0 into t2
    csrwr $t0, save0    # swap save0 and t0 to restore its state

    # t1 = scratch
    # t2 = cpu local
    st.d  $sp, $t2, 24  # save stack pointer into the scratch space
    csrrd $t1, prmd     # check if we came from kernel mode
    andi  $t1, $t1, 0x3 # bits 1:0 = pplv ; 0 = from kernel ; nonzero = from user
    beqz  $t1, 1f       # branch if from kernel mode
    ld.d  $t1, $t2, 0   # load current thread pointer if from user mode
    ld.d  $sp, $t1, 0   # get kernel stack pointer
    1:                  # else proceed

    # generate interrupt frame
    addi.d $sp, $sp, -304
    # save era
    csrrd $t1, era
    st.d  $t1, $sp, 256
    # save prmd
    csrrd $t1, prmd
    st.d  $t1, $sp, 264
    # save estat
    csrrd $t1, estat
    st.d  $t1, $sp, 272
    # save badv
    csrrd $t1, badv
    st.d  $t1, $sp, 280
    # save badi
    csrrd $t1, badi
    st.d  $t1, $sp, 288

    # save sp into frame.x[3]
    ld.d  $t1, $t2, 24
    st.d  $t1, $sp, 24

    # restore t1 and t2 from scratch space
    ld.d  $t1, $t2, 8
    ld.d  $t2, $t2, 16

    # save all registers
    st.d  $r0,  $sp, 0
    st.d  $r1,  $sp, 8
    st.d  $r2,  $sp, 16
    # dont save sp/r3 here
    st.d  $r4,  $sp, 32
    st.d  $r5,  $sp, 40
    st.d  $r6,  $sp, 48
    st.d  $r7,  $sp, 56
    st.d  $r8,  $sp, 64
    st.d  $r9,  $sp, 72
    st.d  $r10, $sp, 80
    st.d  $r11, $sp, 88
    st.d  $r12, $sp, 96
    st.d  $r13, $sp, 104
    st.d  $r14, $sp, 112
    st.d  $r15, $sp, 120
    st.d  $r16, $sp, 128
    st.d  $r17, $sp, 136
    st.d  $r18, $sp, 144
    st.d  $r19, $sp, 152
    st.d  $r20, $sp, 160
    st.d  $r21, $sp, 168
    st.d  $r22, $sp, 176
    st.d  $r23, $sp, 184
    st.d  $r24, $sp, 192
    st.d  $r25, $sp, 200
    st.d  $r26, $sp, 208
    st.d  $r27, $sp, 216
    st.d  $r28, $sp, 224
    st.d  $r29, $sp, 232
    st.d  $r30, $sp, 240
    st.d  $r31, $sp, 248

    # pass the frame as an argument
    move  $a0, $sp

    # ensure cpulocal is valid
    csrrd $r21, save0

    bl dispatch_interrupt
    # disable interrupts
    move  $t0, $zero
    li.d  $t1, (1 << 2)
    csrxchg $t0, $t1, crmd

    # restore required interrupt frame parts
    ld.d  $t0, $sp, 256 # restore era
    csrwr $t0, era
    ld.d  $t0, $sp, 264 # restore prmd
    csrwr $t0, prmd

    # restore all registers
    # skip restoring r0
    ld.d  $r1,  $sp, 8
    ld.d  $r2,  $sp, 16
    # dont restore sp/r3 here
    ld.d  $r4,  $sp, 32
    ld.d  $r5,  $sp, 40
    ld.d  $r6,  $sp, 48
    ld.d  $r7,  $sp, 56
    ld.d  $r8,  $sp, 64
    ld.d  $r9,  $sp, 72
    ld.d  $r10, $sp, 80
    ld.d  $r11, $sp, 88
    ld.d  $r12, $sp, 96
    ld.d  $r13, $sp, 104
    ld.d  $r14, $sp, 112
    ld.d  $r15, $sp, 120
    ld.d  $r16, $sp, 128
    ld.d  $r17, $sp, 136
    ld.d  $r18, $sp, 144
    ld.d  $r19, $sp, 152
    ld.d  $r20, $sp, 160
    ld.d  $r21, $sp, 168
    ld.d  $r22, $sp, 176
    ld.d  $r23, $sp, 184
    ld.d  $r24, $sp, 192
    ld.d  $r25, $sp, 200
    ld.d  $r26, $sp, 208
    ld.d  $r27, $sp, 216
    ld.d  $r28, $sp, 224
    ld.d  $r29, $sp, 232
    ld.d  $r30, $sp, 240
    ld.d  $r31, $sp, 248

    # restore sp from frame.x[3]
    ld.d  $sp, $sp, 24
    ertn
