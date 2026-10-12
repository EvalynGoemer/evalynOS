#pragma once

#include <sched/scheduler.h>

extern void arch_thread_switch(thread_t* prev, thread_t* next);
extern void arch_finalize_user_switch(thread_t* next);

extern void arch_fpu_save(void* fpu_area);
extern void arch_fpu_restore(void* fpu_area);

[[noreturn]] extern void arch_switch_to_user(uint64_t new_ip, uint64_t new_stack);
[[noreturn]] extern void arch_thread_pivot(uint64_t new_ip, uint64_t new_stack);
