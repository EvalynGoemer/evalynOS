#pragma once
#include <stdint.h>
#include <memory/vmm.h>

struct thread {
    int threadId;

    void* stack;
    void* stack_top;
    uint64_t krsp;
    uint64_t ursp;

    pagemap_t* pagemap;

    int is_user_task;
};

struct thread_node {
    struct thread* thread;
    struct thread_node* next_thread;
};

extern void create_thread(void (*entry_point)(void*), pagemap_t *pagemap);
extern void schedule();

struct thread *get_current_thread();
