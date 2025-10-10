#pragma once

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void panic(char* message);
