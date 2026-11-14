.global ap_trampoline_data
.type ap_trampoline_data, @object
.global ap_trampoline
.type ap_trampoline, @function

.extern riscv64_ap_entry

.section .data
.balign 4096
# a0 = hart id
# a1 = per ap data
ap_trampoline:
    # load data from the global ap data
    lla t6, ap_trampoline_data
    ld t0, 0(t6) # t0 = ap entry
    ld t1, 8(t6) # t1 = satp

    # enable paging
    sfence.vma
    csrw satp, t1

    # setup the stack pointer
    mv sp, a1

    # setup cpu local
    ld tp, 0(sp)
    csrw sscratch, tp

    # jump to C
    jr t0

.balign 8
ap_trampoline_data:
    .quad riscv64_ap_entry # offset 0
    .quad 0 # satp         # offset 8
