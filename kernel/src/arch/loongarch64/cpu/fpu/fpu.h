#pragma once
#include <stdint.h>

typedef enum {
    LOONGARCH64_FPU_STD  = 0,
    LOONGARCH64_FPU_LSX  = 1,
    LOONGARCH64_FPU_LASX = 2,
} la64_fpu_t;

#define LOONGARCH64_FPU_CONTROL_AREA_SIZE 16
static const uint16_t la64_fpu_save_sizes[] = {
    [LOONGARCH64_FPU_STD]  = (32 * 8)  + LOONGARCH64_FPU_CONTROL_AREA_SIZE,
    [LOONGARCH64_FPU_LSX]  = (32 * 16) + LOONGARCH64_FPU_CONTROL_AREA_SIZE,
    [LOONGARCH64_FPU_LASX] = (32 * 32) + LOONGARCH64_FPU_CONTROL_AREA_SIZE,
};

static const uint8_t la64_fpu_save_alignments[] = {
    [LOONGARCH64_FPU_STD]  = 8,
    [LOONGARCH64_FPU_LSX]  = 16,
    [LOONGARCH64_FPU_LASX] = 32,
};

extern void setup_fpu();

extern void save_fpu_regs(void* fpu_area);
extern void save_lsx_regs(void* fpu_area);
extern void save_lasx_regs(void* fpu_area);
extern void restore_fpu_regs(void* fpu_area);
extern void restore_lsx_regs(void* fpu_area);
extern void restore_lasx_regs(void* fpu_area);
extern void save_fpu_control(void* fpu_control_area);
extern void restore_fpu_control(void* fpu_control_area);

extern la64_fpu_t la64_best_fpu;
