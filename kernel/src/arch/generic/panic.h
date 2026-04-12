#pragma once

[[noreturn]]
extern void panic(char* message);

extern void panic_print_start(char* message);
extern void panic_print_end();
