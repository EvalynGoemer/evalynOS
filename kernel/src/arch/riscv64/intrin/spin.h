#pragma once

static inline void spin() {
    asm volatile ("pause");
}
