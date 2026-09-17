.extern dispatch_interrupt

.global interrupt_handler_asm
.type interrupt_handler_asm, @function

.equiv crmd,  0x0
.equiv prmd,  0x1
.equiv estat, 0x5
.equiv era,   0x6
.equiv badv,  0x7
.equiv badi,  0x8
.equiv save0, 0x30

.cfi_sections .debug_frame

#define SAVE_REG(reg, sp_off, cfa_off) \
    st.d $r##reg, $sp, sp_off;         \
    .cfi_offset reg, cfa_off

.balign 4096
interrupt_handler_asm:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa 3, 0

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
    ld.d  $sp, $t1, 8   # get kernel stack pointer
    1:                  # else proceed

    # generate interrupt frame
    addi.d $sp, $sp, -304
    .cfi_def_cfa_offset 304

    # save sp into frame.x[3]
    ld.d  $t1, $t2, 24
    st.d  $t1, $sp, 24
    .cfi_offset 3, -280

    # restore t1 and t2 from scratch space
    ld.d  $t1, $t2, 8
    ld.d  $t2, $t2, 16

    # save all registers
    SAVE_REG(0,  0,  -304)
    SAVE_REG(1,  8,  -296)
    SAVE_REG(2,  16, -288)
    # dont save sp/r3 here
    SAVE_REG(4,  32, -272)
    SAVE_REG(5,  40, -264)
    SAVE_REG(6,  48, -256)
    SAVE_REG(7,  56, -248)
    SAVE_REG(8,  64, -240)
    SAVE_REG(9,  72, -232)
    SAVE_REG(10, 80, -224)
    SAVE_REG(11, 88, -216)
    SAVE_REG(12, 96, -208)
    SAVE_REG(13, 104, -200)
    SAVE_REG(14, 112, -192)
    SAVE_REG(15, 120, -184)
    SAVE_REG(16, 128, -176)
    SAVE_REG(17, 136, -168)
    SAVE_REG(18, 144, -160)
    SAVE_REG(19, 152, -152)
    SAVE_REG(20, 160, -144)
    SAVE_REG(21, 168, -136)
    SAVE_REG(22, 176, -128)
    SAVE_REG(23, 184, -120)
    SAVE_REG(24, 192, -112)
    SAVE_REG(25, 200, -104)
    SAVE_REG(26, 208, -96)
    SAVE_REG(27, 216, -88)
    SAVE_REG(28, 224, -80)
    SAVE_REG(29, 232, -72)
    SAVE_REG(30, 240, -64)
    SAVE_REG(31, 248, -56)

    # save era
    csrrd $t0, era
    st.d  $t0, $sp, 256
    .cfi_offset 1, -48
    # save prmd
    csrrd $t1, prmd
    st.d  $t1, $sp, 264
    # save estat
    csrrd $t2, estat
    st.d  $t2, $sp, 272
    # save badv
    csrrd $t3, badv
    st.d  $t3, $sp, 280
    # save badi
    csrrd $t4, badi
    st.d  $t4, $sp, 288

    # ensure cpulocal is valid
    csrrd $r21, save0

    srli.d $t2, $t2, 16      # get ecode from estat
    andi   $t2, $t2, 0x3f    # mask out the non ecode bits
    addi.d $t2, $t2, -0xB    # if t2 (ecode) - 0xB (syscall 0) != 0
    bnez  $t2, .interrupt    # branch to interrupt handler

    addi.d $t0, $t0, 4       # t0 (era) += 4
    st.d  $t0, $sp, 256      # store in frame
    move  $r11, $sp          # pass the frame as the 8th arg
    li.d  $t0, (1 << 2)      # enable interrupts
    csrxchg $t0, $t0, crmd
    bl    dispatch_syscall   # call into C
    st.d  $a0, $sp, 32       # put return values on stack to be restored
    st.d  $a1, $sp, 40
    b     .common

    .interrupt:
    move  $a0, $sp           # pass the frame as an argument
    call dispatch_interrupt

    .common:
    # disable interrupts
    li.d  $t1, (1 << 2)
    csrxchg $zero, $t1, crmd

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
    .cfi_endproc
