#include <loader/elf_structs.h>
#include <loader/symbols.h>
#include <stdint.h>
#include <string.h>

bool symbol_for_pc(const void* symtab, uint64_t symtab_size, const char* strtab, uint64_t strtab_size, uint64_t pc, struct symbol_result* out) {
    if (symtab == nullptr || strtab == nullptr)
        return false;

    const struct elf_symbol_64* symbols = symtab;
    const struct elf_symbol_64* best = nullptr;

    uint64_t symbol_count = symtab_size / sizeof(struct elf_symbol_64);
    for (uint64_t i = 0; i < symbol_count; i++) {
        const struct elf_symbol_64* symbol = &symbols[i];
        if (ELF_ST_TYPE(symbol->info) != ELF_STT_FUNC)
            continue;
        if (symbol->shndx == 0)
            continue;
        if (symbol->value > pc)
            continue;
        if (best == nullptr || symbol->value > best->value)
            best = symbol;
    }

    if (best == nullptr || best->name >= strtab_size)
        return false;

    if (best->size != 0 && pc - best->value >= best->size)
        return false;

    // ensure the string is within the strtab
    const char* name = strtab + best->name;
    uint64_t remaining = (strtab_size - best->name);
    if (strnlen(name, remaining) >= remaining)
        return false;

    out->name   = name;
    out->offset = pc - best->value;
    return true;
}
