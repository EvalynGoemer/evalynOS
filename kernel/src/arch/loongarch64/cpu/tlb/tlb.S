#include <arch/loongarch64/intrin/paging.h>
#include <arch/loongarch64/intrin/csr.h>

.global setup_mmu
.type setup_mmu, @function
setup_mmu:
    li.d  $t0,  LA64_PWCL       # load constants into t0 -> t3
    li.d  $t1,  LA64_PWCH
    li.d  $t2,  PAGE_SIZE_SHIFT
    li.d  $t3,  PAGE_SIZE_SHIFT
    csrwr $t0, CSR_PWCL         # load the constants into CSRs
    csrwr $t1, CSR_PWCH
    csrwr $t2, CSR_STLBPS
    csrwr $t3, CSR_TLBREHI
    csrwr $a0, CSR_PGDH         # set new page tables
    csrwr $a1, CSR_TLBRENTRY    # set tlb handler addr
    invtlb 0, $r0, $r0
    jr $ra

.global tlb_refill_handler
.type tlb_refill_handler, @function
.balign 4096
tlb_refill_handler:
    csrwr $t0, CSR_TLBRSAVE # save t0
    csrrd $t0, CSR_PGD      # get the faulting page table root
    lddir $t0, $t0, 3       # walk page table level 4
    lddir $t0, $t0, 2       # walk page table level 3
#if LA64_PAGING_LEVELS >= 4
    lddir $t0, $t0, 1       # walk page table level 2
#endif
    ldpte $t0, 0            # load even entry from level 1
    ldpte $t0, 1            # load odd entry from level 1
    csrrd $t0, CSR_TLBRSAVE # restore t0
    tlbfill                 # fill the tlb
    ertn
