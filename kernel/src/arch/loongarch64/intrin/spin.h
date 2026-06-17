#pragma once

static inline void spin() {
    asm volatile ("ibar 0");
}
