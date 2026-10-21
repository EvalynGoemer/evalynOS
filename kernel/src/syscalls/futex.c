#include <arch/generic/paging/paging.h>
#include <sched/wait.h>
#include <syscalls/syscalls.h>
#include <utils/dstruct/rbtree.h>
#include <utils/lib.h>
#include <utils/locks/ticketlock.h>
#include <stdint.h>
#include <arch/intrin/cpulocal.h>
#include <mem/address_space.h>
#include <stdlib.h>
#include <string.h>

// TODO: handle pthreads properly
// TODO: handle fork properly

// while this is worthless for threading due to the todos i do not want to stub
// futexes and at least get the basic structure for them since c++ needs them
// even for non threaded things. pthreads will need the address space to get
// locked properly (which is missing in other parts too) and fork/cow will need
// futexes to check if a region is private and put it under the addrspace
// instead of going through the paddr. however the basic structure is *fine*
// barring some optimizations to be made

typedef struct {
    uint64_t paddr;
    rbtree_node_t linkage;
    waitable_t waitable;
    uint32_t refcnt;
} futex_entry_t;

typedef struct {
    futex_entry_t* entry;
    uint32_t expected;
} futex_token_t;

static uint64_t futex_tree_von(rbtree_node_t* node) {
    futex_entry_t* entry = CONTAINER_OF(node, futex_entry_t, linkage);
    return entry->paddr;
}

static bool futex_wait_cond(void* ctx) {
    futex_token_t* token = ctx;
    uint32_t* kptr = TO_HHDM_PTR(token->entry->paddr);
    uint32_t val = __atomic_load_n(kptr, __ATOMIC_RELAXED);
    return val != token->expected;
}

static rbtree_t futex_tree = {.value_of_node = futex_tree_von, .root = nullptr};
static ticketlock_t futex_tree_lock = {0};

sysret_t syscall_futex_wait(uint32_t* uptr, uint32_t expected, uint64_t timeout) {
    if (!IS_ALIGNED(uptr, alignof(uint32_t))) return SYSRET_EINVAL;

    address_space_t* addrspace = CPU_LOCAL_GET_CURRENT_THREAD()->addrspace;
    uint64_t prot = paging_get_prot(addrspace->pagetable, (uint64_t)uptr);
    if (!(prot & PAGE_U)) return SYSRET_EINVAL;

    uint64_t paddr = paging_get_paddr(addrspace->pagetable, (uint64_t)uptr, nullptr);
    uint64_t offset = MISALIGNMENT(uptr, PAGE_SIZE);
    uint64_t key = paddr + offset;

    ticketlock_lock(&futex_tree_lock);
    rbtree_node_t* node = rbtree_search(&futex_tree, key, RB_SEARCH_TYPE_EXACT);

    if (node == nullptr) {
        futex_entry_t* entry = kmalloc(sizeof(futex_entry_t));
        memset(entry, 0, sizeof(futex_entry_t));
        entry->paddr = key;
        node = &entry->linkage;
        rbtree_insert(&futex_tree, node);
    }

    futex_entry_t* entry = CONTAINER_OF(node, futex_entry_t, linkage);
    __atomic_fetch_add(&entry->refcnt, 1, __ATOMIC_ACQUIRE);
    ticketlock_unlock(&futex_tree_lock);

    futex_token_t token = {.entry = entry, .expected = expected};
    wait_status_t status = wait_on_cond(&entry->waitable, &futex_wait_cond, &token, timeout);

    ticketlock_lock(&futex_tree_lock);
    if (__atomic_sub_fetch(&entry->refcnt, 1, __ATOMIC_RELEASE) == 0) {
        rbtree_remove(&futex_tree, &entry->linkage);
        kfree(entry, sizeof(futex_entry_t));
    }
    ticketlock_unlock(&futex_tree_lock);

    switch (status) {
        case WAIT_STATUS_WOKEN: return SYSRET_OKAY;
        case WAIT_STATUS_TIMED_OUT: return SYSRET_ETIMEDOUT;
        case WAIT_STATUS_CONDITION_MET: return SYSRET_EAGAIN;
    }
}

sysret_t syscall_futex_wake(uint32_t* uptr, int count) {
    if (!IS_ALIGNED(uptr, alignof(uint32_t))) return SYSRET_EINVAL;

    address_space_t* addrspace = CPU_LOCAL_GET_CURRENT_THREAD()->addrspace;
    uint64_t prot = paging_get_prot(addrspace->pagetable, (uint64_t)uptr);
    if (!(prot & PAGE_U)) return SYSRET_EINVAL;

    uint64_t paddr = paging_get_paddr(addrspace->pagetable, (uint64_t)uptr, nullptr);
    uint64_t offset = MISALIGNMENT(uptr, PAGE_SIZE);
    uint64_t key = paddr + offset;

    ticketlock_lock(&futex_tree_lock);
    rbtree_node_t* node = rbtree_search(&futex_tree, key, RB_SEARCH_TYPE_EXACT);

    if (node == nullptr) {
        ticketlock_unlock(&futex_tree_lock);
        return (sysret_t){.retval = 0, .error = 0};
    }

    futex_entry_t* entry = CONTAINER_OF(node, futex_entry_t, linkage);
    int woken = sched_wakeup_n_threads(&entry->waitable, count);
    ticketlock_unlock(&futex_tree_lock);

    return (sysret_t){.retval = woken, .error = 0};
}
