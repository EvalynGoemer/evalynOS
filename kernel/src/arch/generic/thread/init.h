#pragma once

#include <stdint.h>

extern uintptr_t arch_prepare_thread_stack(uintptr_t stack_top, uintptr_t entry, uintptr_t arg);
