#pragma once

#include <arch/x86_64/cpu/interrupts.h>

[[noreturn]]
extern void panic_interrupt(const char* message, interrupt_frame_t* frame);
