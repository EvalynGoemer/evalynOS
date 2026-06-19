#pragma once

#include <stdint.h>

typedef struct interrupt_frame {
    uint64_t x[32];
    uint64_t era;
    uint64_t prmd;
    uint64_t estat;
    uint64_t badv;
    uint64_t badi;
} interrupt_frame_t;

extern void dispatch_interrupt(interrupt_frame_t* frame);
extern void setup_interrupts();

extern void interrupt_handler_asm();

extern const char* exception_names[];
