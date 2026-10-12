#include <utils/lib.h>
#define ZERO_TO_7 0,1,2,3,4,5,6,7

.global save_fpu_control
.type save_fpu_control, @function
.global restore_fpu_control
.type restore_fpu_control, @function
.global init_fpu_state
.type init_fpu_state, @function

#define GEN_FPU_FUNC(NAME, REGS, ...)            ;\
    .global NAME                                 ;\
    .type NAME, @function                        ;\
    NAME:                                        ;\
    ITER_REGS(REGS ## __VA_OPT__(), __VA_ARGS__) ;\
    ret                                          ;\
    .size NAME, . - NAME

GEN_FPU_FUNC(zero_fpu_regs, ZERO_TO_31, movgr2fr.d $f\i, $zero)
GEN_FPU_FUNC(zero_lsx_regs, ZERO_TO_31, vxor.v $vr\i, $vr\i, $vr\i)
GEN_FPU_FUNC(zero_lasx_regs, ZERO_TO_31, xvxor.v $xr\i, $xr\i, $xr\i)
GEN_FPU_FUNC(save_fpu_regs, ZERO_TO_31, fst.d $f\i, $a0, (\i * 8))
GEN_FPU_FUNC(save_lsx_regs, ZERO_TO_31, vst $vr\i, $a0, (\i * 16))
GEN_FPU_FUNC(save_lasx_regs, ZERO_TO_31, xvst $xr\i, $a0, (\i * 32))
GEN_FPU_FUNC(restore_fpu_regs, ZERO_TO_31, fld.d $f\i, $a0, (\i * 8))
GEN_FPU_FUNC(restore_lsx_regs, ZERO_TO_31, vld $vr\i, $a0, (\i * 16))
GEN_FPU_FUNC(restore_lasx_regs, ZERO_TO_31, xvld $xr\i, $a0, (\i * 32))

save_fpu_control:
    movfcsr2gr $t0, $fcsr0
    st.d       $t0, $a0, 0
    move       $t1, $zero
    ITER_REGS(ZERO_TO_7, movcf2gr $t0, $fcc\i ; bstrins.d $t1, $t0, \i, \i)
    st.d $t1, $a0, 8
    ret

restore_fpu_control:
    ld.d $t1, $a0, 8
    ITER_REGS(ZERO_TO_7, bstrpick.d $t0, $t1, \i, \i ; movgr2cf $fcc\i, $t0)
    ld.d       $t0, $a0, 0
    movgr2fcsr $fcsr0, $t0
    ret

.section .rodata
.balign 8
zero_fpu_jtable:
    .dword zero_fpu_regs  - zero_fpu_jtable
    .dword zero_lsx_regs  - zero_fpu_jtable
    .dword zero_lasx_regs - zero_fpu_jtable

.section .text
init_fpu_state:
    move $t0, $ra                  # zero_***_regs dosnt clobber t0

    la.pcrel $t1, la64_best_fpu    # get the current fpu
    ld.w     $t1, $t1, 0           # load into t1
    slli.d   $t1, $t1, 3           # multiply by 8 to get index into jump table

    la.pcrel $t2, zero_fpu_jtable  # get the jump table base into t2
    add.d    $t1, $t2, $t1         # base + index = jump table entry into t1
    ld.d     $t1, $t1, 0           # load from the entry into t1
    add.d    $t1, $t2, $t1         # function is at base + offset
    jirl     $ra, $t1, 0           # call the function from the jump table

    # clear FPU control state
    movgr2fcsr $fcsr0, $zero
    ITER_REGS(ZERO_TO_7, movgr2cf $fcc\i, $zero)

    move $ra, $t0                  # restore ra
    ret                            # return
