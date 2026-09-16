#include <stdint.h>
#include <string.h>
#include <arch/generic/paging/paging.h>
#include <arch/intrin/loader.h>
#include <loader/dwarf/cfi.h>
#include <loader/dwarf/helpers.h>
#include <mem/memmap.h>
#include <utils/lib.h>
#include <utils/limine.h>

// TODO: maybe make this a public paging helper instead
static bool dwarf_vaddr_readable(uint64_t vaddr, uint64_t size) {
    #ifdef ARCH_UNPAGED_HHDM
    uint64_t hhdm_base = hhdm_request.response->offset;
    if (vaddr >= hhdm_base && vaddr - hhdm_base + size <= memmap_last_paddr())
        return true;
    #endif

    // if paging hasn't been set up it's not safe to traverse page tables
    if (!kernel_page_table) return false;

    // check the page tables to make sure the pages that will be used are valid
    if (paging_get_paddr(kernel_page_table, vaddr, nullptr) == 0) return false;
    if (MISALIGNMENT(vaddr, PAGE_SIZE) + size > PAGE_SIZE) {
        if (paging_get_paddr(kernel_page_table, vaddr + size - 1, nullptr) == 0) return false;
    }
    return true;
}

// helper for advancing the loc in some opcodes
static bool cfi_advance(uint64_t* cur, uint64_t delta, uint64_t code_align, uint64_t target) {
    uint64_t next = *cur + delta * code_align;
    if (next > target) return false;
    *cur = next;
    return true;
}

#define DW_CFA_advance_loc      0x40
#define DW_CFA_offset           0x80
#define DW_CFA_restore          0xC0
#define DW_CFA_nop              0x00
#define DW_CFA_advance_loc1     0x02
#define DW_CFA_advance_loc2     0x03
#define DW_CFA_advance_loc4     0x04
#define DW_CFA_remember_state   0x0A
#define DW_CFA_restore_state    0x0B
#define DW_CFA_def_cfa          0x0C
#define DW_CFA_def_cfa_register 0x0D
#define DW_CFA_def_cfa_offset   0x0E

bool dwarf_cfi_execute(struct cfi_state* state, struct dwarf_cie* cie, uint8_t* opcodes_start, uint8_t* opcodes_end, uint64_t loc) {
    uint8_t* p = opcodes_start;
    uint8_t* end = opcodes_end;
    uint64_t cur = 0;

    while (p < end) {
        uint8_t opcode = *p++;
        uint64_t operandA, operandB;

        // opcodes that have any of the upper two bits set
        // store operandA in the lower six bits
        if ((opcode & 0xC0) != 0) {
            operandA = opcode & ~0xC0;
            opcode &= 0xC0;
        }

        switch (opcode) {
            // these opcodes advance the location counter and tell us when to stop
            case DW_CFA_advance_loc: {
                if (!cfi_advance(&cur, operandA, cie->code_align, loc))
                    return true;
                break;
            }
            case DW_CFA_advance_loc1:
            case DW_CFA_advance_loc2:
            case DW_CFA_advance_loc4: {
                // this works because the value of these opcodes line up well
                operandB = 1 << (opcode - DW_CFA_advance_loc1);

                if (!dwarf_read_bytes(&p, end, operandB, &operandA))
                    return false;
                if (!cfi_advance(&cur, operandA, cie->code_align, loc))
                    return true;
                break;
            }

            // restore the state to how the cie left it
            // if the cie happens to call it zero it out
            case DW_CFA_restore: {
                if (operandA >= DWARF_MAX_REGS) break;
                if (cie->opcodes_start == opcodes_start)
                    state->rules[state->depth].regs[operandA] = (struct cfi_rule){0};
                else
                    state->rules[state->depth].regs[operandA] = state->cfi_baseline.regs[operandA];
                break;
            }

            // push the current state on the stack
            case DW_CFA_remember_state: {
                if (state->depth + 1 >= CFI_STACK_DEPTH_MAX) return false;
                state->rules[state->depth + 1] = state->rules[state->depth];
                state->depth++;
                break;
            }

            // pop the previous state from the stack
            case DW_CFA_restore_state: {
                if (state->depth == 0) return false;
                state->depth--;
                break;
            }

            // this is used to define the CFA rules for specific registers
            case DW_CFA_offset: {
                if (!dwarf_read_uleb(&p, end, &operandB)) return false;
                if (operandA < DWARF_MAX_REGS) {
                    struct cfi_rule* r = &state->rules[state->depth].regs[operandA];
                    r->kind   = CFI_OFFSET;
                    r->offset = (int64_t)operandB * cie->data_align;
                }
                break;
            }

            // these are used to define the CFA rules for the main CFA value
            case DW_CFA_def_cfa: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                if (!dwarf_read_uleb(&p, end, &operandB)) return false;
                if (operandA >= DWARF_MAX_REGS) return false;
                struct cfi_rule* cfa = &state->rules[state->depth].cfa;
                cfa->kind = CFI_OFFSET;
                cfa->reg_num = operandA;
                cfa->offset = operandB;
                break;
            }
            case DW_CFA_def_cfa_register: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                if (operandA >= DWARF_MAX_REGS) return false;
                state->rules[state->depth].cfa.reg_num = operandA;
                break;
            }
            case DW_CFA_def_cfa_offset: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                state->rules[state->depth].cfa.offset = operandA;
                break;
            }

            // do you really need a comment for this?
            case DW_CFA_nop: break;

            // TODO: implement more opcodes
            default: return false;
        }
    }

    return true;
}

// this is the function that actually handles and processes all the rules
bool dwarf_cfi_resolve(struct cfi_state* state) {
    struct cfi_rules* rules = &state->rules[state->depth];
    struct cfi_rule* cfa = &rules->cfa;

    if (cfa->kind != CFI_OFFSET) return false;
    uint64_t cfa_value = state->regs[cfa->reg_num] + (uint64_t)cfa->offset;
    state->regs[ARCH_DWARF_SP_REG] = cfa_value;

    for (int i = 0; i < DWARF_MAX_REGS; i++) {
        struct cfi_rule* reg = &rules->regs[i];
        switch (reg->kind) {
            // the register is at a plain offset from the cfa
            case CFI_OFFSET: {
                uint64_t vaddr = cfa_value + (uint64_t)reg->offset;
                if (!dwarf_vaddr_readable(vaddr, sizeof(uint64_t))) return false;
                uint64_t val = 0;
                memcpy(&val, (void*)vaddr, sizeof(val));
                state->regs[i] = val;
                break;
            }
            case CFI_SAME_VALUE: break;
            default: return false;
        }
    }

    return true;
}

bool dwarf_cfi(void* section, uint64_t section_length, uint64_t pc, struct cfi_state* state) {
    struct dwarf_fde fde;
    struct dwarf_cie cie;

    if (!dwarf_get_fde(section, section_length, pc, &fde, &cie))
        return false;

    // update here since dwarf_get_fde dosnt take the state and passing
    // that through is more convoluted than just passing it like this
    state->is_signal_frame = cie.is_signal_frame;

    state->depth = 0;
    state->ra_col = cie.ra_col;
    memset(state->rules, 0, sizeof(state->rules));

    // execute the cie
    if (!dwarf_cfi_execute(state, &cie, cie.opcodes_start, cie.opcodes_end, 0))
        return false;

    // save the initial cie state for the DW_CFA_restore opcode
    state->cfi_baseline = state->rules[state->depth];

    // execute the actual fde
    if (!dwarf_cfi_execute(state, &cie, fde.opcodes_start, fde.opcodes_end, pc - fde.initial_location))
        return false;

    return true;
}
