#pragma once
#include <stdint.h>
#include "sched/scheduler.h"

extern uint32_t total_cpus;
extern uint32_t started_cpus;
extern uint64_t ap_init_start_time;

extern void setup_aps();
extern void arch_init_aps();
extern void arch_reclaim_ap_memory();

extern uint64_t allocate_cpu_local();
extern thread_t* allocate_init_thread();
