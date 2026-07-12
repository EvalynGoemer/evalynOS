#pragma once

#include <utils/dstruct/llist.h>
#include <stdint.h>

typedef enum: uint32_t {
    THREAD_RUNNING,
    THREAD_RUNABLE,
    THREAD_BLOCKED,
    THREAD_REAPING,
} thread_state_t;

typedef struct thread {
    uintptr_t kstack;
    uintptr_t kstack_alloc_base;
    uintptr_t kstack_alloc_size;
    void* fpu_save;
    uintptr_t user_stack_save;

    uint32_t preempt_disable_counter;
    thread_state_t state;
    llist_node_t node;
} thread_t;

extern void early_sched_init();
extern void schedule();
extern void schedule_finalize(thread_t* prev, thread_t* next);
