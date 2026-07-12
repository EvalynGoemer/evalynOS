#pragma once

#include <sched/scheduler.h>

extern void arch_thread_switch(thread_t* prev, thread_t* next);
