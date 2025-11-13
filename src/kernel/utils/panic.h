#pragma once
#include "interupts/interupts.h"

#define PANIC_FLAGS_VECTOR (1 << 0)
#define PANIC_FLAGS_FRAME (1 << 1)
#define PANIC_FLAGS_ERROR (1 << 2)

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void panic(char* message, int vector, struct interrupt_frame* frame, unsigned long error, int flags);
