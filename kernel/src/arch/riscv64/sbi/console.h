#pragma once
#include "sbi.h"

static inline long sbi_console_putchar(int c) {
    return legacy_sbi_ecall1(LEGACY_SBI_CONSOLE_PUTCHAR, c);
}

static inline long sbi_console_getchar() {
    return legacy_sbi_ecall0(LEGACY_SBI_CONSOLE_GETCHAR);
}
