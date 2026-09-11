#pragma once

#include <utils/dstruct/llist.h>
#include <stdint.h>

// forward decls
struct address_space;

typedef enum: uint32_t {
    THREAD_RUNNING,
    THREAD_RUNABLE,
    THREAD_BLOCKED,
    THREAD_REAPING,
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
} thread_t;

_Static_assert(offsetof(thread_t, kstack) == 0);
_Static_assert(offsetof(thread_t, user_stack_save) == 32);

extern void early_sched_init();
extern void schedule();
extern void schedule_finalize(thread_t* prev, thread_t* next);
extern void enqueue_thread(thread_t* thread);
