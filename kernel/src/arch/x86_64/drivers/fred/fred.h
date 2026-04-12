#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <arch/x86_64/cpu/interrupts.h>

typedef struct fred_frame {
    irq_saved_regs_t regs;
    irq_cpu_frame_t cpu_frame;
    uint64_t fred_event_data;
    uint64_t fred_reserved;
} fred_frame_t;

extern bool fred_enabled;

extern bool setup_fred_bsp();

extern void fred_switch_to_user(uint64_t start_addr, uint64_t stack_top);

extern void fred_ring3_entry(fred_frame_t* frame);
extern void fred_ring0_entry(fred_frame_t* frame);
extern void fred_ring3_entry_asm_stub();
