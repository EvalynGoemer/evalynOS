#pragma once
#include <stdint.h>

typedef struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector, error;
    uint64_t ip, cs, flags, rsp, ss;
} interrupt_frame_t;

extern void dispatch_interrupt(interrupt_frame_t *frame);
extern const char *exception_names[];

#define INTERRUPT_DEBUG_TRAP 0x01
#define INTERRUPT_NON_MASKABLE_INTERRUPT 0x02
#define INTERRUPT_BREAKPOINT_TRAP 0x03
#define INTERRUPT_DOUBLE_FAULT 0x08
#define INTERRUPT_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_PAGE_FAULT 0x0E
