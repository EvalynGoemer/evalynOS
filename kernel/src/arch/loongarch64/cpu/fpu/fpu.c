#include <arch/loongarch64/cpu/fpu/fpu.h>
#include <arch/loongarch64/cpu/cpucfg.h>
#include <arch/loongarch64/intrin/csr.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

la64_fpu_t la64_best_fpu;

void setup_fpu(bool bsp) {
    assert(cpucfg_check(CPUCFG_HAS_FPU));

    uint64_t euen = csrrd(CSR_EUEN);

    if (bsp == true) {
        la64_best_fpu = LOONGARCH64_FPU_STD;
        euen |= CSR_EUEN_FPU;

        if (cpucfg_check(CPUCFG_HAS_LSX)) {
            LOG_TAGGED_OK("FPU", ANSI_BRED, "LSX Enabled")
            la64_best_fpu = LOONGARCH64_FPU_LSX;
            euen |= CSR_EUEN_LSX;
        }

        if (cpucfg_check(CPUCFG_HAS_LASX)) {
            LOG_TAGGED_OK("FPU", ANSI_BRED, "LASX Enabled")
            la64_best_fpu = LOONGARCH64_FPU_LASX;
            euen |= CSR_EUEN_LASX;
        }
    } else {
        switch (la64_best_fpu) {
            case LOONGARCH64_FPU_LASX: euen |= CSR_EUEN_LASX; [[fallthrough]];
            case LOONGARCH64_FPU_LSX:  euen |= CSR_EUEN_LSX;  [[fallthrough]];
            case LOONGARCH64_FPU_STD:  euen |= CSR_EUEN_FPU;  break;
        }
    }

    csrwr(CSR_EUEN, euen);
}
