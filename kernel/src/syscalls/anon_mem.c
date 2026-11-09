#include <stddef.h>
#include <stdint.h>
#include <syscalls/syscalls.h>
#include <syscalls/handles.h>
#include <arch/generic/paging/paging.h>
#include <arch/intrin/cpulocal.h>
#include <mem/pmm.h>
#include <mem/address_space.h>
#include <mem/vmem.h>

static inline sysret_t anon_alloc_internal(size_t size, uint64_t addr, int attr, address_space_t* addrspace) {
    size = ALIGN_UP(size, PAGE_SIZE);
    uint64_t mem = vmem_alloc(addrspace->valloc, size, addr);
    if (mem == 0) return SYSRET_ENOMEM;

    uint64_t pages = size >> PAGE_SIZE_SHIFT;
    for (uint64_t i = 0; pages > i; i++) {
        uint64_t vaddr = mem + (i * PAGE_SIZE);
        uint64_t paddr = pmm_alloc_page();
        paging_map_page(addrspace->pagetable, vaddr, paddr, attr);
    }

    return (sysret_t){.retval = mem, .error = 0};
}

static inline sysret_t anon_free_internal(size_t base, size_t len, address_space_t* addrspace) {
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

sysret_t syscall_anon_alloc(size_t size) {
    return anon_alloc_internal(size, 0, PAGE_URW, CPU_LOCAL_GET_CURRENT_THREAD()->addrspace);
}

sysret_t syscall_anon_free(size_t base, size_t len) {
    return anon_free_internal(base, len, CPU_LOCAL_GET_CURRENT_THREAD()->addrspace);
}

sysret_t syscall_anon_alloc_remote(size_t size, size_t addr, int attr, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_ADDRSPACE);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;

    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE)
        return SYSRET_EINVAL;

    return anon_alloc_internal(size, addr, attr, handle->addrspace);
}

sysret_t syscall_anon_free_remote(size_t base, size_t len, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_ADDRSPACE);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;

    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE)
        return SYSRET_EINVAL;

    return anon_free_internal(base, len, handle->addrspace);
}
