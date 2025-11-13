#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <scheduler/scheduler.h>
#include <scheduler/switch.h>
#include <filesystem/filesystem.h>
#include <memory/vmm.h>
#include <memory/pmm.h>
#include <drivers/x86_64/ports.h>
#include <utils/panic.h>

uint64_t STACK_SIZE = 65536;

void task_quit() {
    while (1) {

    }
}

struct thread_node* threads = NULL;

void create_thread(void (*entry_point)(void*)) {
    bool had_threads = (threads != NULL);

    struct thread* new_thread = malloc(sizeof(struct thread));
    memset(new_thread, 0, sizeof(struct thread));

    new_thread->stack = malloc(STACK_SIZE);
    memset(new_thread->stack, 0, STACK_SIZE);
    new_thread->stack_top = (void *)(((uintptr_t)new_thread->stack + STACK_SIZE) & ~0xFULL);

    new_thread->is_user_task = 0;

    uint64_t *stack = (uint64_t *)new_thread->stack_top;

    if (entry_point == NULL) {
        entry_point = task_quit;
    }

    *--stack = (uint64_t)task_quit;
    *--stack = (uint64_t)entry_point;
    *--stack = 0x202;

    for (int i = 0; i < 15; i++) {
        *--stack = 0x0 + i;
    }

    new_thread->rsp = (uint64_t)stack;

    struct thread_node* new_node = malloc(sizeof(struct thread_node));
    new_node->thread = new_thread;

    if (!had_threads) {
        new_node->next_thread = new_node;
        threads = new_node;
    } else {
        new_node->next_thread = threads->next_thread;
        threads->next_thread = new_node;
    }

    shouldSchedule = 1;
}

void schedule() {
    outb(0x20, 0x20);

    struct thread *previous_thread = threads->thread;
    threads = threads->next_thread;
    struct thread *current_thread = threads->thread;

    thread_switch(&previous_thread->rsp, current_thread->rsp);
}
