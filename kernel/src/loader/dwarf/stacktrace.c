#include <stdint.h>
#include <stdio.h>
#include <arch/intrin/loader.h>
#include <loader/dwarf/line.h>
#include <loader/dwarf/stacktrace.h>
#include <loader/elf_introspection.h>
#include <loader/symbols.h>

// use BSS here since it's large and shouldn't be on the stack
// as well we can't use the heap in a panic situation
static struct cfi_state cfi_state;

void dwarf_stack_trace(interrupt_frame_t* frame) {
    // TODO: maybe use an ELF file with just the debug info passed as a module
    // instead of always embedding it into the main binary and using elf introspection

    uint64_t debug_frame_size = 0, debug_line_size = 0, symtab_size = 0, strtab_size = 0;
    void* debug_frame = elf_introspect_section(".debug_frame", &debug_frame_size);
    void* debug_line  = elf_introspect_section(".debug_line", &debug_line_size);
    void* symtab_section = elf_introspect_section(".symtab", &symtab_size);
    void* strtab_section = elf_introspect_section(".strtab", &strtab_size);

    if (!debug_frame) return;
    if (!debug_line) return;
    if (!symtab_section) return;
    if (!strtab_section) return;

    // the first frame is always a "signal frame"
    cfi_state.is_signal_frame = true;

    arch_cfi_seed(&cfi_state, frame);
    uint64_t pc_rt = arch_cfi_initial_pc(frame);

    printf("\033[38;2;120;200;255mStack Trace:\n");

    for (int i = 0; i < 8; i++) {
        uint64_t pc_lt = kaslr_rt2lt(pc_rt - (cfi_state.is_signal_frame ? 0 : 1));

        printf("  - 0x%016lx", pc_lt);

        struct symbol_result symbol;
        if (symbol_for_pc(symtab_section, symtab_size, strtab_section, strtab_size, pc_lt, &symbol))
            printf(" %s+0x%lx", symbol.name, symbol.offset);

        struct dwarf_line_result line;
        if (dwarf_line_for_pc(debug_line, debug_line_size, pc_lt, &line)) {
            if (line.dir != nullptr)
                printf(" (%s/%s:%lu)", line.dir, line.file, line.line);
            else
                printf(" (%s:%lu)", line.file, line.line);
        }

        printf("\n");

        if (!dwarf_cfi(debug_frame, debug_frame_size, pc_lt, &cfi_state)) break;
        if (!dwarf_cfi_resolve(&cfi_state)) break;
        if (cfi_state.ra_col >= DWARF_MAX_REGS) break;

        pc_rt = cfi_state.regs[cfi_state.ra_col];
        if (pc_rt == 0) break;
    }
}
