#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <arch/x86_64/cpu/interrupts.h>

#define FRED_FAKE_VECTOR_CPL0   0
#define FRED_FAKE_VECTOR_CPL3   3

#define FRED_EVENT_TYPE_EXTINT  0
#define FRED_EVENT_TYPE_NMI     2
#define FRED_EVENT_TYPE_HWFAULT 3
#define FRED_EVENT_TYPE_SWINT   4
#define FRED_EVENT_TYPE_KSWINT  5
#define FRED_EVENT_TYPE_SWFAULT 6
#define FRED_EVENT_TYPE_SYSCALL 7

extern bool fred_enabled;

extern bool setup_fred_bsp();

extern void fred_switch_to_user(uint64_t start_addr, uint64_t stack_top);

extern void fred_ring3_entry_asm();
