.extern dispatch_interrupt
.global interrupt_handler_asm
.balign 4
interrupt_handler_asm:
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
    ld sp, 0(t1)            # get kernel stack pointer
    1:                      # else proceed

    # generate interrupt frame
    addi sp, sp, -288
    # save sepc
    csrr t1, sepc
    sd t1, 256(sp)
    # save sstatus
    csrr t1, sstatus
    sd t1, 264(sp)
    # save scause
    csrr t1, scause
    sd t1, 272(sp)
    # save stval
    csrr t1, stval
    sd t1, 280(sp)

    # save sp into frame.x[2]
    ld t1, 24(t0)
    sd t1, 16(sp)

    # restore t0 and t1 from scratch space
    ld t1,   16(t0)
    ld t0,    8(t0)

    # save all registers
    sd x0,    0(sp)
    sd x1,    8(sp)
    # dont save sp/x2 here
    sd x3,   24(sp)
    sd x4,   32(sp)
    sd x5,   40(sp)
    sd x6,   48(sp)
    sd x7,   56(sp)
    sd x8,   64(sp)
    sd x9,   72(sp)
    sd x10,  80(sp)
    sd x11,  88(sp)
    sd x12,  96(sp)
    sd x13, 104(sp)
    sd x14, 112(sp)
    sd x15, 120(sp)
    sd x16, 128(sp)
    sd x17, 136(sp)
    sd x18, 144(sp)
    sd x19, 152(sp)
    sd x20, 160(sp)
    sd x21, 168(sp)
    sd x22, 176(sp)
    sd x23, 184(sp)
    sd x24, 192(sp)
    sd x25, 200(sp)
    sd x26, 208(sp)
    sd x27, 216(sp)
    sd x28, 224(sp)
    sd x29, 232(sp)
    sd x30, 240(sp)
    sd x31, 248(sp)

    # pass the frame as an argument
    mv a0, sp

    # ensure cpulocal is valid
    csrr tp, sscratch

    call dispatch_interrupt
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
