#pragma once

#include <stdint.h>

typedef struct interrupt_frame {
    uint64_t x[32];
    uint64_t sepc;
    uint64_t sstatus;
    uint64_t scause;
    uint64_t stval;
} interrupt_frame_t;

extern void dispatch_interrupt(interrupt_frame_t* frame);
extern void setup_interrupts();

extern void interrupt_handler_asm();

extern const char* exception_names[];
