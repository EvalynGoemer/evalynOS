#pragma once
#include <arch/x86_64/cpu/interrupts.h>
#include <stdint.h>

extern void setup_syscall();
extern void syscall_handler();
