#pragma once

#include <stdint.h>

typedef struct cpucfg_request {
    uint32_t index;
    uint8_t bit;
} cpucfg_request_t;

static inline uint32_t cpucfg(uint32_t index) {
    uint32_t value;
    asm volatile ("cpucfg %0, %1" : "=r"(value) : "r"(index));
    return value;
}

static inline bool cpucfg_check(cpucfg_request_t req) {
    uint32_t value = cpucfg(req.index);
    return value & (1ull << req.bit);
}

// values from https://loongson.github.io/LoongArch-Documentation/LoongArch-Vol1-EN.html#_cpucfg

#define CPUCFG_INDEX_TIMER_FREQ 4
#define CPUCFG_INDEX_TIMER_MUDV 5

#define CPUCFG_HAS_TIMER ((cpucfg_request_t){2, 14})

// technically DP floats but this is mandated to be on par with the rv64gc kernel requirements
// hopefully no CPU is insane to have DP floats but not SP / normal floats :clueless:
#define CPUCFG_HAS_FPU   ((cpucfg_request_t){1, 2})
#define CPUCFG_HAS_LSX   ((cpucfg_request_t){1, 6})
#define CPUCFG_HAS_LASX  ((cpucfg_request_t){1, 7})
