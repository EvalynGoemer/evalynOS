#include "stddef.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <scheduler/scheduler.h>
#include <scheduler/switch.h>
#include <filesystem/filesystem.h>
#include <memory/vmm.h>
#include <memory/pmm.h>
#include <drivers/x86_64/ports.h>
#include <drivers/x86_64/gdt.h>
#include <drivers/x86_64/msr.h>
#include <utils/panic.h>

uint64_t STACK_SIZE = 65536;

void task_quit() {
    while (1) {

    }
}

struct thread_node* threads = NULL;
int next_thread_id = 0;

void create_thread(void (*entry_point)(void*), pagemap_t *pagemap) {
    bool had_threads = (threads != NULL);

    struct thread* new_thread = malloc(sizeof(struct thread));
    memset(new_thread, 0, sizeof(struct thread));

    new_thread->stack = malloc(STACK_SIZE);
    memset(new_thread->stack, 0, STACK_SIZE);
    new_thread->stack_top = (void *)(((uintptr_t)new_thread->stack + STACK_SIZE) & ~0xFULL);

    new_thread->is_user_task = 0;
    new_thread->threadId = next_thread_id;
    next_thread_id++;

    if (pagemap != NULL) {
        new_thread->pagemap = pagemap;
    } else {
        new_thread->pagemap = kernel_pagemap;
    }

    uint64_t *stack = (uint64_t *)new_thread->stack_top;

    if (entry_point == NULL) {
        entry_point = task_quit;
    }
    *--stack = (uint64_t)task_quit;
    *--stack = (uint64_t)entry_point;
    *--stack = 0x202;

    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;

    new_thread->krsp = (uint64_t)stack;

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

    asm volatile("cli");

    struct thread *previous_thread = threads->thread;
    threads = threads->next_thread;
    struct thread *current_thread = threads->thread;

    vmm_switch_to(current_thread->pagemap);

    tss.rsp0 = (uint64_t)current_thread->stack_top;
    wrmsr(UGSBAS, (uint64_t)current_thread);

    thread_switch(&previous_thread->krsp, current_thread->krsp);
}

struct thread *get_current_thread() {
    return threads->thread;
}
