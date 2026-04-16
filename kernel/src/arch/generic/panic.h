#pragma once

[[noreturn]]
extern void panic(const char* message);

extern void panic_print_start(const char* message);
extern void panic_print_end();
