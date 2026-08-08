.equiv tlbrsave, 0x8B
.equiv pgd,      0x1B

.global tlb_refill_handler
.balign 4096
tlb_refill_handler:
    csrwr $t0, tlbrsave  # save t0
    csrrd $t0, pgd       # get the faulting page table root
    lddir $t0, $t0, 3    # walk page table level 4
    lddir $t0, $t0, 2    # walk page table level 3
    lddir $t0, $t0, 1    # walk page table level 2
    ldpte $t0, 0         # load even entry from level 1
    ldpte $t0, 1         # load odd entry from level 1
    tlbfill              # fill the tlb
    csrrd $t0, tlbrsave  # restore t0
    ertn
