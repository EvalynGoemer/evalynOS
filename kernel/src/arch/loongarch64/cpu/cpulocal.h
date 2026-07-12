#pragma once

#include <utils/locks/spinlock.h>
#include <utils/dstruct/llist.h>
#include <stdint.h>
#include <stddef.h>

typedef struct fixed_cpu_local {
    void*     current_thread;
    uint64_t  scratch[3];
    llist_t   run_queue;
    spinlock_t sched_lock;
} fixed_cpu_local_t;

_Static_assert(offsetof(fixed_cpu_local_t, current_thread) == 0);
_Static_assert(offsetof(fixed_cpu_local_t, scratch) == 8);
_Static_assert(offsetof(fixed_cpu_local_t, run_queue) == 32);
_Static_assert(offsetof(fixed_cpu_local_t, sched_lock) == 56);
_Static_assert(sizeof(fixed_cpu_local_t) <= 2048);

[[gnu::section(".cpu_local_fixed")]]
extern fixed_cpu_local_t fixed_cpu_local;

extern void setup_cpulocal_bsp();
