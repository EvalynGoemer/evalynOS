.globl kentry
.type kentry, @function

.extern kmain
.extern setup_dmw
.extern rebase_limine_requests
.extern hhdm_request

kentry:
    la.pcrel $t0, hhdm_request # get the hhdm request
    ld.d     $s0, $t0, 40      # get the response pointer
    ld.d     $s1, $s0, 8       # get the old hhdm offset

    bl setup_dmw               # setup DMW registers
    bl rebase_limine_requests  # rebase limine requests

    ld.d  $s0, $s0, 8          # get the new hhdm offset
    sub.d $t0, $s0, $s1        # calculate the delta
    add.d $sp, $sp, $t0        # make the sp use the new hhdm

    move  $ra, $zero           # do not permit returning back
    b kmain                    # jump to kmain
