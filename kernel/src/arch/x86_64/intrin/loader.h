#pragma once

#include <stdint.h>

#include <arch/x86_64/cpu/interrupts.h>
#include <loader/dwarf/cfi.h>
#include <loader/elf_structs.h>

#define ELF_CURRENT_ARCH  ELF_X86_64_ARCH
#define ARCH_DWARF_SP_REG 7
#define ARCH_DWARF_RA_REG 16

static inline void arch_cfi_seed(struct cfi_state* state, interrupt_frame_t* frame) {
    state->regs[0] = frame->regs.rax;
    state->regs[1] = frame->regs.rdx;
    state->regs[2] = frame->regs.rcx;
    state->regs[3] = frame->regs.rbx;
    state->regs[4] = frame->regs.rsi;
    state->regs[5] = frame->regs.rdi;
    state->regs[6] = frame->regs.rbp;
    state->regs[7] = frame->cpu_frame.rsp;
    state->regs[8] = frame->regs.r8;
    state->regs[9] = frame->regs.r9;
    state->regs[10] = frame->regs.r10;
    state->regs[11] = frame->regs.r11;
    state->regs[12] = frame->regs.r12;
    state->regs[13] = frame->regs.r13;
    state->regs[14] = frame->regs.r14;
    state->regs[15] = frame->regs.r15;
    state->regs[ARCH_DWARF_RA_REG] = frame->cpu_frame.ip;
}

static inline uint64_t arch_cfi_initial_pc(interrupt_frame_t* frame) {
    return frame->cpu_frame.ip;
}
