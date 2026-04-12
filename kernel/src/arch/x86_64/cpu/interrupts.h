#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct irq_saved_regs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
} irq_saved_regs_t;

typedef struct irq_cpu_frame {
    uint64_t error, ip, cs, flags, rsp, ss;
} irq_cpu_frame_t;

typedef struct interrupt_frame {
    irq_saved_regs_t regs;
    uint64_t vector;
    irq_cpu_frame_t cpu_frame;
} interrupt_frame_t;

// asserts for assembly
_Static_assert(offsetof(interrupt_frame_t, regs)      == 0,   "bad regs offset");
_Static_assert(offsetof(interrupt_frame_t, vector)    == 120, "bad vector offset");
_Static_assert(offsetof(interrupt_frame_t, cpu_frame) == 128, "bad cpu_frame offset");

extern void dispatch_interrupt(irq_saved_regs_t* regs, irq_cpu_frame_t* frame, uint64_t vector);
extern const char *exception_names[];

#define INTERRUPT_DEBUG_TRAP 0x01
#define INTERRUPT_NON_MASKABLE_INTERRUPT 0x02
#define INTERRUPT_BREAKPOINT_TRAP 0x03
#define INTERRUPT_DOUBLE_FAULT 0x08
#define INTERRUPT_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_PAGE_FAULT 0x0E
