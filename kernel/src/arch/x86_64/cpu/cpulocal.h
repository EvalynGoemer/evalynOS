#pragma once

#include <stdint.h>

typedef struct fixed_cpu_local {
    void* current_thread;
    uint64_t scratch[1];
} fixed_cpu_local_t;

[[gnu::section(".cpu_local_fixed")]]
extern fixed_cpu_local_t fixed_cpu_local;

extern void setup_cpulocal_bsp();
