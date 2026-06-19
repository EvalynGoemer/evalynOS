#pragma once
#include <stdint.h>

[[noreturn]] extern void panic(const char* message);

typedef struct interrupt_frame interrupt_frame_t;
[[noreturn]] extern void panic_interrupt(const char* message, interrupt_frame_t* frame);

extern void panic_print_start(const char* message);
extern void panic_print_end();

extern uint32_t panic_flag;
