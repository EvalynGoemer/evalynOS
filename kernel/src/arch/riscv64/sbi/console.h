#pragma once
#include "sbi.h"

#define SBI_EXT_CONSOLE_PUTCHAR   0x01
#define SBI_EXT_CONSOLE_GETCHAR   0x02

static inline long sbi_console_putchar(int c) {
    return sbi_ecall1(SBI_EXT_CONSOLE_PUTCHAR, 0, c).error;
}

static inline long sbi_console_getchar(void) {
    sbiret_t ret = sbi_ecall0(SBI_EXT_CONSOLE_GETCHAR, 0);
    return ret.error ? ret.error : ret.value;
}
