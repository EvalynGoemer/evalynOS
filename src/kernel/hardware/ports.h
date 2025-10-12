#pragma once
#include <stdint.h>

#include "../libc/stdio.h"

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void outb(uint16_t port, uint8_t val);

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
uint8_t inb(uint16_t port);
