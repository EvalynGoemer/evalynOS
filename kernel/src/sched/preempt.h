#pragma once

#include <arch/intrin/cpulocal.h>
#include <sched/scheduler.h>

static inline void enable_preemption() {
    thread_t* current = CPU_LOCAL_GET_CURRENT_THREAD();
    current->preempt_disable_counter--;
}

static inline void disable_preemption() {
    thread_t* current = CPU_LOCAL_GET_CURRENT_THREAD();
    current->preempt_disable_counter++;
}
