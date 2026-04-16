#pragma once

#include <arch/x86_64/cpu/interrupts.h>
#include <stdint.h>

[[noreturn]]
extern void panic_interrupt(const char* message, irq_saved_regs_t* regs, irq_cpu_frame_t* frame, uint64_t vector);
