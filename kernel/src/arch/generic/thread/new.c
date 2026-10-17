#include "arch/generic/thread/switch.h"
#include "sched/scheduler.h"
#include <arch/generic/thread/init.h>
#include <arch/intrin/cpulocal.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils/locks/irqlock.h>
#include <utils/defer.h>
#include <mem/pmm.h>
#include <loader/elf.h>
#include <mem/address_space.h>
#include <stdlib.h>
#include <utils/misc/ustar.h>

thread_t* create_kthread(uintptr_t entry, size_t arg1, size_t arg2) {
    uintptr_t paddr = pmm_alloc_page();
    thread_t* thread = malloc(sizeof(thread_t));
    memset(thread, 0, sizeof(thread_t));
    thread->kstack_top  = TO_HHDM(paddr + PAGE_SIZE);
    thread->kstack_size = PAGE_SIZE;
    thread->kstack = arch_prepare_thread_stack(TO_HHDM(paddr) + PAGE_SIZE, entry, arg1, arg2);
    thread->state = THREAD_RUNABLE;
    thread->addrspace = nullptr;
    return thread;
}

thread_t* create_uthread(uintptr_t entry, uintptr_t ustack, address_space_t* addrspace) {
    thread_t* thread = create_kthread((uintptr_t)arch_switch_to_user, entry, ustack);
    thread->fpu_save = arch_allocate_fpu_area();
    thread->addrspace = addrspace;
    thread->user_stack_save = ustack;
    return thread;
}

thread_t* create_uthread_from_elf(const char* initramfs_path) {
    if (!initramfs) return nullptr;

    void* elf = ustar_lookup(initramfs, initramfs_path, nullptr);
    if (!elf) return nullptr;

    address_space_t* as = new_address_space();
    uint64_t entry = load_elf(elf, as);

    if (entry == 0) {
        free_address_space(as);
        return nullptr;
    }

    uint64_t stack_base = vmem_alloc(as->valloc, 0x20000, 0);
    uint64_t stack_top = stack_base + 0x20000;
    for (uint64_t a = stack_base; a < stack_top; a += PAGE_SIZE)
        paging_map_page(as->pagetable, a, pmm_alloc_page(), PAGE_URW);

    uint64_t  stack_paddr = paging_get_paddr(as->pagetable, stack_top - PAGE_SIZE, nullptr);
    uintptr_t ustack_top = TO_HHDM(stack_paddr) + PAGE_SIZE;
    uintptr_t ustack = ustack_top;

    ustack -= 8; *(uintptr_t*)ustack = 0; // auxv
    ustack -= 8; *(uintptr_t*)ustack = 0; // envp
    ustack -= 8; *(uintptr_t*)ustack = 0; // argv
    ustack -= 8; *(uintptr_t*)ustack = 0; // argc

    uint64_t sp = stack_top - (ustack_top - ustack);
    return create_uthread(entry, sp, as);
}
