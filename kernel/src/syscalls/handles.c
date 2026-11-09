#include "sched/scheduler.h"
#include "syscalls/syscalls.h"
#include <stddef.h>
#include <utils/lib.h>
#include <syscalls/handles.h>
#include <arch/generic/thread/new.h>
#include <mem/address_space.h>
#include <utils/locks/ticketlock.h>
#include <utils/dstruct/rbtree.h>
#include <arch/intrin/cpulocal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint64_t next_id = 0;
#define GET_NEXT_ID() (__atomic_fetch_add(&next_id, 1, __ATOMIC_RELAXED))

handle_t* new_handle() {
    handle_t* handle = kmalloc(sizeof(handle_t));
    memset(handle, 0, sizeof(handle_t));
    SET_HANDLE_ID(handle->idtype, GET_NEXT_ID());

    rbtree_t* handle_tree = &CPU_LOCAL_GET_CURRENT_THREAD()->addrspace->handles;
    rbtree_insert(handle_tree, &handle->node);

    return handle;
}

handle_t* find_handle(uint64_t id) {
    rbtree_t* handle_tree = &CPU_LOCAL_GET_CURRENT_THREAD()->addrspace->handles;
    rbtree_node_t* node = rbtree_search(handle_tree, id, RB_SEARCH_TYPE_EXACT);
    if (!node) return nullptr;
    return CONTAINER_OF(node, handle_t, node);
}

void free_handle(handle_t* handle) {
    rbtree_t* handle_tree = &CPU_LOCAL_GET_CURRENT_THREAD()->addrspace->handles;
    rbtree_remove(handle_tree, &handle->node);
    kfree(handle, sizeof(handle_t));
}

sysret_t syscall_new_addrspace() {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_ADDRSPACE);
    handle_t* handle = new_handle();
    SET_HANDLE_TYPE(handle->idtype, HANDLE_ADDRSPACE);
    handle->addrspace = new_address_space();
    return (sysret_t){.retval = GET_HANDLE_ID(handle->idtype), .error = 0};
}

// TODO: make handling of other things more proper so this wont need to invalidate the handle to be safe
// mainly pages on pmap need to be refcounted so there wont be UAF issues, a mapping *can* stay active
// after calling this but this prevents making new ones until that is fixed
sysret_t syscall_start_addrspace(size_t ip, size_t sp, uint64_t handle_id) {
    SYSCALL_REQUIRE_PERM(SYSCALL_PERM_ADDRSPACE);
    handle_t* handle = find_handle(handle_id);
    if (!handle) return SYSRET_EINVAL;
    if (GET_HANDLE_TYPE(handle->idtype) != HANDLE_ADDRSPACE) return SYSRET_EINVAL;

    thread_t* thread = create_uthread(ip, sp, handle->addrspace);
    enqueue_thread(thread);
    free_handle(handle);
    return SYSRET_OKAY;
}
