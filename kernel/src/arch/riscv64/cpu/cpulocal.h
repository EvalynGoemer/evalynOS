#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct fixed_cpu_local {
    void*     current_thread;
    uint64_t  scratch[3];
} fixed_cpu_local_t;

_Static_assert(offsetof(fixed_cpu_local_t, current_thread) == 0);
_Static_assert(offsetof(fixed_cpu_local_t, scratch) == 8);

[[gnu::section(".cpu_local_fixed")]]
extern fixed_cpu_local_t fixed_cpu_local;

extern void setup_cpulocal_bsp();
