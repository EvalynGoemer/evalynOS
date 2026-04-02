#pragma once

[[noreturn]]
extern void panic(char* message);

[[noreturn]]
extern void panic_interrupt(char* message, void* interrupt_frame);

extern void panic_print_start(char* message);
extern void panic_print_end();
