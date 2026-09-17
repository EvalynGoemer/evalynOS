.extern dispatch_interrupt
.extern dispatch_syscall
.global vectored_interrupt_table

.global interrupt_handler_asm
.type interrupt_handler_asm, @function

.option push
.option norvc
.balign 4096
vectored_interrupt_table:
.rept 32
    j interrupt_handler_asm
.endr
.option pop

.cfi_sections .debug_frame

#define SAVE_REG(reg, sp_off, cfa_off) \
    sd x##reg, sp_off(sp);             \
    .cfi_offset reg, cfa_off

.balign 4
interrupt_handler_asm:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa 2, 0

    # cpulocal + 0  = current thread struct
    # cpulocal + 8  = scratch t0
    # cpulocal + 16 = scratch t1
    # cpulocal + 24 = scratch sp
    # current thread + 0 = ksp
    csrrw t0, sscratch, t0  # swap sscratch and t0 to get cpu local
    sd t1, 16(t0)           # save t1 into cpu local
    csrrw t1, sscratch, t0  # put cpu local back into sscratch and put old t0 into t1
    sd t1, 8(t0)            # save t0 into cpu local

    # t0 = cpu local
    # t1 = scratch
    sd sp, 24(t0)           # save stack pointer into the scratch space
    csrr t1, sstatus        # check if we came from kernel mode
    andi t1, t1, (1 << 8)   # bit 8 = SPP ; nonzero = from kernel ; 0 = from user
    bnez t1, 1f             # branch if from kernel mode
    ld t1, 0(t0)            # load current thread pointer if from user mode
    ld sp, 8(t1)            # get kernel stack pointer
    1:                      # else proceed

    # generate interrupt frame
    addi sp, sp, -288
    .cfi_def_cfa_offset 288

    # save sp into frame.x[2]
    ld t1, 24(t0)
    sd t1, 16(sp)
    .cfi_offset 2, -272

    # restore t0 and t1 from scratch space
    ld t1,   16(t0)
    ld t0,    8(t0)

    # save all registers
    SAVE_REG(0,  0,   -288)
    SAVE_REG(1,  8,   -280)
    # dont save sp/x2 here
    SAVE_REG(3,  24,  -264)
    SAVE_REG(4,  32,  -256)
    SAVE_REG(5,  40,  -248)
    SAVE_REG(6,  48,  -240)
    SAVE_REG(7,  56,  -232)
    SAVE_REG(8,  64,  -224)
    SAVE_REG(9,  72,  -216)
    SAVE_REG(10, 80,  -208)
    SAVE_REG(11, 88,  -200)
    SAVE_REG(12, 96,  -192)
    SAVE_REG(13, 104, -184)
    SAVE_REG(14, 112, -176)
    SAVE_REG(15, 120, -168)
    SAVE_REG(16, 128, -160)
    SAVE_REG(17, 136, -152)
    SAVE_REG(18, 144, -144)
    SAVE_REG(19, 152, -136)
    SAVE_REG(20, 160, -128)
    SAVE_REG(21, 168, -120)
    SAVE_REG(22, 176, -112)
    SAVE_REG(23, 184, -104)
    SAVE_REG(24, 192, -96)
    SAVE_REG(25, 200, -88)
    SAVE_REG(26, 208, -80)
    SAVE_REG(27, 216, -72)
    SAVE_REG(28, 224, -64)
    SAVE_REG(29, 232, -56)
    SAVE_REG(30, 240, -48)
    SAVE_REG(31, 248, -40)

    # save sepc
    csrr t0, sepc
    sd t0, 256(sp)
    .cfi_offset 1, -32
    # save sstatus
    csrr t1, sstatus
    sd t1, 264(sp)
    # save scause
    csrr t2, scause
    sd t2, 272(sp)
    # save stval
    csrr t3, stval
    sd t3, 280(sp)

    # ensure cpulocal is valid
    csrr tp, sscratch

    addi t2, t2, -8         # if t2 (scause) - 8 (ecall from umode) != 0
    bnez t2, .interrupt     # branch to interrupt handler
    addi t0, t0, 4          # t0 (sepc) += 4
    sd t0, 256(sp)          # store in frame
    mv a7, sp               # pass the frame as the 8th arg
    csrsi sstatus, 2        # enable interrupts
    call dispatch_syscall   # call into C
    sd a0, 80(sp)           # put return values on stack to be restored
    sd a1, 88(sp)
    j .common

    .interrupt:
    mv a0, sp               # pass the frame as an argument
    call dispatch_interrupt

    .common:
    # disable interrupts
    csrci sstatus, 2

    # restore required interrupt frame parts
    ld t0, 256(sp)    # restore sepc
    csrw sepc, t0
    ld t0, 264(sp)    # restore sstatus
    csrw sstatus, t0

    # restore all registers
    # skip restoring x0
    ld x1,    8(sp)
    # dont restore sp/x2 here
    ld x3,   24(sp)
    ld x4,   32(sp)
    ld x5,   40(sp)
    ld x6,   48(sp)
    ld x7,   56(sp)
    ld x8,   64(sp)
    ld x9,   72(sp)
    ld x10,  80(sp)
    ld x11,  88(sp)
    ld x12,  96(sp)
    ld x13, 104(sp)
    ld x14, 112(sp)
    ld x15, 120(sp)
    ld x16, 128(sp)
    ld x17, 136(sp)
    ld x18, 144(sp)
    ld x19, 152(sp)
    ld x20, 160(sp)
    ld x21, 168(sp)
    ld x22, 176(sp)
    ld x23, 184(sp)
    ld x24, 192(sp)
    ld x25, 200(sp)
    ld x26, 208(sp)
    ld x27, 216(sp)
    ld x28, 224(sp)
    ld x29, 232(sp)
    ld x30, 240(sp)
    ld x31, 248(sp)

    # restore sp from frame.x[2]
    ld sp, 16(sp)
    sret
    .cfi_endproc
