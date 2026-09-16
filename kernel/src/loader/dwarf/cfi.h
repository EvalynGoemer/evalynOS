#pragma once

#include <stdint.h>
#include <loader/dwarf/fde.h>

// riscv currently only uses one layer of nesting so a max depth of 2 is fine
// other archs currently do not need nesting
#define CFI_STACK_DEPTH_MAX 2

#define DWARF_MAX_REGS 32

enum cfi_rule_kind {
    CFI_SAME_VALUE,
    CFI_OFFSET,
};

struct cfi_rule {
    enum cfi_rule_kind kind;
    uint8_t reg_num;
    int64_t offset;
};

struct cfi_rules {
    struct cfi_rule cfa;
    struct cfi_rule regs[DWARF_MAX_REGS];
};

struct cfi_state {
    struct cfi_rules cfi_baseline;
    struct cfi_rules rules[CFI_STACK_DEPTH_MAX];
    uint64_t         regs[DWARF_MAX_REGS];
    uint64_t         ra_col;
    uint8_t          depth;
    bool             is_signal_frame;
};

extern bool dwarf_cfi_execute(struct cfi_state* state, struct dwarf_cie* cie, uint8_t* opcodes_start, uint8_t* opcodes_end, uint64_t loc);
extern bool dwarf_cfi(void* section, uint64_t section_length, uint64_t pc, struct cfi_state* state);
extern bool dwarf_cfi_resolve(struct cfi_state* state);
