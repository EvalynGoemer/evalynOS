#pragma once

#include <stdint.h>

struct symbol_result {
    const char* name;
    uint64_t    offset;
};

extern bool symbol_for_pc(const void* symtab, uint64_t symtab_size, const char* strtab, uint64_t strtab_size, uint64_t pc, struct symbol_result* out);
