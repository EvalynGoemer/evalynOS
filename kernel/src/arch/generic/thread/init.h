#pragma once

#include <stdint.h>

extern uintptr_t arch_prepare_thread_stack(uintptr_t stack_top, uintptr_t entry, uintptr_t arg1, uintptr_t arg2);

extern void* arch_allocate_fpu_area();
extern void arch_free_fpu_area(void* fpu_area);
