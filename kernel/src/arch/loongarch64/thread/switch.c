#include <arch/loongarch64/cpu/fpu/fpu.h>

void arch_fpu_save(void* fpu_area) {
    switch (la64_best_fpu) {
        case LOONGARCH64_FPU_STD:  save_fpu_regs(fpu_area);  break;
        case LOONGARCH64_FPU_LSX:  save_lsx_regs(fpu_area);  break;
        case LOONGARCH64_FPU_LASX: save_lasx_regs(fpu_area); break;
    }
    save_fpu_control(fpu_area + la64_fpu_save_sizes[la64_best_fpu] - LOONGARCH64_FPU_CONTROL_AREA_SIZE);
}

void arch_fpu_restore(void* fpu_area) {
    switch (la64_best_fpu) {
        case LOONGARCH64_FPU_STD:  restore_fpu_regs(fpu_area);  break;
        case LOONGARCH64_FPU_LSX:  restore_lsx_regs(fpu_area);  break;
        case LOONGARCH64_FPU_LASX: restore_lasx_regs(fpu_area); break;
    }
    restore_fpu_control(fpu_area + la64_fpu_save_sizes[la64_best_fpu] - LOONGARCH64_FPU_CONTROL_AREA_SIZE);
}
