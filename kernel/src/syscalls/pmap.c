#include "mem/vmem.h"
#include "syscalls/syscalls.h"
#include "utils/lib.h"
#include <arch/intrin/cpulocal.h>
#include <syscalls/handles.h>
#include <mem/address_space.h>
#include <arch/generic/paging/paging.h>
#include <stddef.h>
#include <stdint.h>

static inline sysret_t pmap_internal(uint64_t paddr, size_t size, int flags, address_space_t* addrspace) {
    paddr = ALIGN_DOWN(paddr, PAGE_SIZE);
    size = ALIGN_UP(size, PAGE_SIZE);
    uint64_t addr = vmem_alloc(addrspace->valloc, size, 0);
    vmem_find_segment(addrspace->valloc, addr)->flags |= VMEM_FLAGS_NO_FREE;
    paging_map_range(addrspace->pagetable, addr, paddr, size, flags);
    return (sysret_t){.retval = addr, .error = 0};
}

static inline sysret_t punmap_internal(size_t base, size_t size, address_space_t* addrspace) {
    base = ALIGN_DOWN(base, PAGE_SIZE);
    size = ALIGN_UP(size, PAGE_SIZE);
    paging_unmap_range(addrspace->pagetable, base, size);
    vmem_free(addrspace->valloc, base, size);
    return SYSRET_OKAY;
}

sysret_t syscall_pmap(uint64_t paddr, size_t size, int flags) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
    return pmap_internal(paddr, size, flags, CPU_LOCAL_GET_CURRENT_THREAD()->addrspace);
}

sysret_t syscall_punmap(size_t base, size_t size) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
    return punmap_internal(base, size, CPU_LOCAL_GET_CURRENT_THREAD()->addrspace);
}

sysret_t syscall_pmap_remote(uint64_t paddr, size_t size, int flags, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;

    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE)
        return SYSRET_EINVAL;

    return pmap_internal(paddr, size, flags, handle->addrspace);
}

sysret_t syscall_punmap_remote(size_t base, size_t size, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;

    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE)
        return SYSRET_EINVAL;

    return punmap_internal(base, size, handle->addrspace);
}

sysret_t syscall_pmap_remote_vaddr(size_t remote_addr, size_t size, int flags, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_PMAP);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;

    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE)
        return SYSRET_EINVAL;

    address_space_t* addrspace = CPU_LOCAL_GET_CURRENT_THREAD()->addrspace;
    address_space_t* remote_addrspace = handle->addrspace;

    uint64_t addr = vmem_alloc(addrspace->valloc, size, 0);
    vmem_find_segment(addrspace->valloc, addr)->flags |= VMEM_FLAGS_NO_FREE;

    remote_addr = ALIGN_DOWN(remote_addr, PAGE_SIZE);
    size = ALIGN_UP(size, PAGE_SIZE);

    uint64_t pages = size >> PAGE_SIZE_SHIFT;
    for (uint64_t i = 0; pages > i; i++) {
        uint64_t vaddr = addr + (i * PAGE_SIZE);
        uint64_t remote_vaddr = remote_addr + (i * PAGE_SIZE);
        uint64_t paddr = paging_get_paddr(remote_addrspace->pagetable, remote_vaddr, nullptr);
        paging_map_page(addrspace->pagetable, vaddr, paddr, flags);
    }

    return (sysret_t){.retval = addr, .error = 0};
}
