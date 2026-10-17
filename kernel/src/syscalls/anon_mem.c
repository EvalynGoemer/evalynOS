#include <stddef.h>
#include <syscalls/syscalls.h>
#include <arch/generic/paging/paging.h>
#include <arch/intrin/cpulocal.h>
#include <mem/pmm.h>
#include <mem/address_space.h>
#include <mem/vmem.h>

sysret_t syscall_anon_alloc(size_t size) {
    address_space_t* addrspace = CPU_LOCAL_GET_CURRENT_THREAD()->addrspace;
    size = ALIGN_UP(size, PAGE_SIZE);
    uint64_t mem = vmem_alloc(addrspace->valloc, size, 0);
    if (mem == 0) return SYSRET_ENOMEM;

    uint64_t pages = size >> PAGE_SIZE_SHIFT;
    for (uint64_t i = 0; pages > i; i++) {
        uint64_t vaddr = mem + (i * PAGE_SIZE);
        uint64_t paddr = pmm_alloc_page();
        paging_map_page(addrspace->pagetable, vaddr, paddr, PAGE_URW);
    }

    return (sysret_t){.retval = mem, .error = 0};
}

sysret_t syscall_anon_free(size_t base, size_t len) {
    address_space_t* addrspace = CPU_LOCAL_GET_CURRENT_THREAD()->addrspace;
    base = ALIGN_DOWN(base, PAGE_SIZE);
    len = ALIGN_UP(len, PAGE_SIZE);

    uint64_t pages = len >> PAGE_SIZE_SHIFT;
    for (uint64_t i = 0; pages > i; i++) {
        uint64_t vaddr = base + (i * PAGE_SIZE);
        uint64_t paddr = paging_get_paddr(addrspace->pagetable, vaddr, nullptr);
        if (paddr == 0) continue;
        paging_unmap_page(addrspace->pagetable, vaddr, PAGE_NORM);
        pmm_free_page(paddr);
    }

    vmem_free(addrspace->valloc, base, len);

    return SYSRET_OKAY;
}
