#pragma once
#include <stdint.h>

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
    // only safe to access when FRED is enabled
    uint64_t fred_extra, fred_reserved;
} interrupt_frame_t;

extern void dispatch_interrupt(interrupt_frame_t* frame);
extern const char *exception_names[];

#define INTERRUPT_DEBUG_TRAP 0x01
#define INTERRUPT_NON_MASKABLE_INTERRUPT 0x02
#define INTERRUPT_BREAKPOINT_TRAP 0x03
#define INTERRUPT_DOUBLE_FAULT 0x08
#define INTERRUPT_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_MACHINE_CHECK_EXCEPTION 0x12
#define INTERRUPT_PAGE_FAULT 0x0E

extern void handle_exception_mce(interrupt_frame_t* frame);
