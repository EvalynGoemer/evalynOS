#pragma once
#include <stdint.h>

struct thread {
    int threadId;

    void* stack;
    void* stack_top;
    uint64_t rsp;

    int is_user_task;
};

struct thread_node {
    struct thread* thread;
    struct thread_node* next_thread;
};

extern void setup_threading();
extern void create_thread(void (*entry_point)(void*));
extern void schedule();
