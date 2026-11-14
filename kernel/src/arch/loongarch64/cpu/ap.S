#include <arch/loongarch64/intrin/paging.h>
#include <arch/loongarch64/intrin/iocsr.h>
#include <arch/loongarch64/intrin/csr.h>
#include <arch/loongarch64/cpu/dmw.h>

.global ap_trampoline_data
.type ap_trampoline_data, @object
.global ap_trampoline
.type ap_trampoline, @function

.extern loongarch64_ap_entry

.section .data
.balign PAGE_SIZE

# NOTE: this isnt documented to be proper but edk2 passes mailbox3 into a1
# and clears the other mailboxes so we dont have a choice but to do it this way
# https://github.com/tianocore/edk2/blob/8628578dacd38b327d4f9acf820c6914f4b3a48f/OvmfPkg/LoongArchVirt/Sec/LoongArch64/Start.S#L164
# a1 = per ap data (from mailbox 3)
ap_trampoline:
    # load data from the global ap data
    la.pcrel $t8, ap_trampoline_data
    ld.d $t0, $t8, 0  # t0 = entry
    ld.d $t1, $t8, 8  # t1 = tlb handler
    ld.d $t2, $t8, 16 # t2 = page tables

    # setup paging CSRs
    li.d  $t3, LA64_PWCL        # load constants into t3 -> t6
    li.d  $t4, LA64_PWCH
    li.d  $t5, PAGE_SIZE_SHIFT
    li.d  $t6, PAGE_SIZE_SHIFT
    csrwr $t3, CSR_PWCL         # load the constants into CSRs
    csrwr $t4, CSR_PWCH
    csrwr $t5, CSR_STLBPS
    csrwr $t6, CSR_TLBREHI
    csrwr $t1, CSR_TLBRENTRY    # set tlb handler addr

    # set page tables
    move $t1, $t2
    csrwr $t1, CSR_PGDH
    csrwr $t2, CSR_PGDL

    # setup DMWs
    li.d $t1, (0x8 << DMW_VSEG_SHIFT) | DMW_MAT_CACHED | DMW_PLV0
    li.d $t2, (0x9 << DMW_VSEG_SHIFT) | DMW_MAT_CACHED | DMW_PLV0
    li.d $t3, (0xA << DMW_VSEG_SHIFT) | DMW_MAT_WEAK_UNCACHED | DMW_PLV0
    li.d $t4, (0xB << DMW_VSEG_SHIFT) | DMW_MAT_STRONG_UNCACHED | DMW_PLV0
    csrwr $t1, CSR_DMW0
    csrwr $t2, CSR_DMW1
    csrwr $t3, CSR_DMW2
    csrwr $t4, CSR_DMW3

    # enable CRMD.PG and disable CRMD.DA
    li.d    $t1, 0b10000
    li.d    $t2, 0b11000
    csrxchg $t1, $t2, CSR_CRMD

    # setup stack
    move $sp, $a1

    # setup cpulocal
    ld.d $t1, $a1, 0
    move $r21, $t1
    csrwr $t1, CSR_SAVE0

    # jump to C and pass per cpu data
    move $a0, $a1
    jr $t0

.balign 8
ap_trampoline_data:
    .quad loongarch64_ap_entry # offset 0
    .quad 0 # tlb handler      # offset 8
    .quad 0 # PGDH             # offset 16
