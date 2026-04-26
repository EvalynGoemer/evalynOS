#pragma once

#include <stdint.h>

typedef struct interrupt_frame {
    uint64_t x[32];
} interrupt_frame_t;
