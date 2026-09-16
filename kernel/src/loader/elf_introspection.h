#pragma once

#include <stdint.h>

extern uint64_t elf_introspect_base();
extern uint64_t elf_introspect_top();
extern uint64_t elf_introspect_kaslr_slide();
extern void* elf_introspect_section(const char* name, uint64_t* size);

#define kaslr_rt2lt(a) ((a) - elf_introspect_kaslr_slide())
#define kaslr_lt2rt(a) ((a) + elf_introspect_kaslr_slide())
