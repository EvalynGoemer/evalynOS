#pragma once

#include <stdint.h>
#include <string.h>

#include <arch/riscv64/cpu/interrupts.h>
#include <loader/dwarf/cfi.h>
#include <loader/elf_structs.h>

#define ELF_CURRENT_ARCH  ELF_RISCV_ARCH
#define ARCH_DWARF_SP_REG 2
#define ARCH_DWARF_RA_REG 1

static inline void arch_cfi_seed(struct cfi_state* state, interrupt_frame_t* frame) {
    memcpy(state->regs, frame->x, sizeof(frame->x));
}

static inline uint64_t arch_cfi_initial_pc(interrupt_frame_t* frame) {
    return frame->sepc;
}
