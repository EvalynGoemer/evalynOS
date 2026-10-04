#pragma once

#include "utils/dstruct/pheap.h"
#include <utils/dstruct/llist.h>
#include <stdint.h>

// forward decls
struct address_space;

typedef enum: uint32_t {
    THREAD_RUNNING,
    THREAD_RUNABLE,
    THREAD_REAPING,
    THREAD_BLOCKING,
    THREAD_BLOCKED,
    THREAD_IDLE_THREAD,
} thread_state_t;

typedef struct thread {
    uintptr_t kstack;
    uintptr_t kstack_top;
    uintptr_t kstack_size;
    void* fpu_save;
    uintptr_t user_stack_save;

    struct address_space* addrspace;
    uint32_t preempt_disable_counter;
    thread_state_t state;
    llist_node_t node;

    uint64_t wakeup_time;
    pheap_node_t timeout_pheap_node;
} thread_t;

_Static_assert(offsetof(thread_t, kstack) == 0);
_Static_assert(offsetof(thread_t, user_stack_save) == 32);

extern void early_sched_init_bsp();
extern void early_sched_init_ap(thread_t* init_thread, uint64_t idle_stack, uint64_t idle_stack_size);
extern void sched_init();
extern void schedule();
extern void schedule_finalize(thread_t* prev, thread_t* next);
extern void enqueue_thread(thread_t* thread);
