#pragma once
#include <stdint.h>

[[noreturn]]
extern void panic(const char* message);

extern void panic_print_start(const char* message);
extern void panic_print_end();

extern uint32_t panic_flag;
