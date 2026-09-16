#include <stdint.h>
#include <string.h>

#include <loader/dwarf/helpers.h>
#include <loader/dwarf/line.h>

#define DW_LNS_copy               0x01
#define DW_LNS_advance_pc         0x02
#define DW_LNS_advance_line       0x03
#define DW_LNS_set_file           0x04
#define DW_LNS_set_column         0x05
#define DW_LNS_negate_stmt        0x06
#define DW_LNS_set_basic_block    0x07
#define DW_LNS_const_add_pc       0x08
#define DW_LNS_fixed_advance_pc   0x09
#define DW_LNS_set_prologue_end   0x0A
#define DW_LNS_set_epilogue_begin 0x0B
#define DW_LNS_set_isa            0x0C

#define DW_LNE_end_sequence       0x01
#define DW_LNE_set_address        0x02
#define DW_LNE_define_file        0x03
#define DW_LNE_set_discriminator  0x04

struct line_vm {
    uint64_t address;
    uint64_t line;
    uint64_t file;
    uint64_t cand_line;
    uint64_t cand_file;
};

static bool skip_string(uint8_t** p, uint8_t* end) {
    size_t len = strnlen((const char*)*p, (end - *p));
    if (*p + len >= end) return false;
    *p += len + 1;
    return true;
}

static bool emit_row(struct line_vm* vm, uint64_t pc) {
    if (vm->address > pc)
        return true;
    vm->cand_line = vm->line;
    vm->cand_file = vm->file;
    return false;
}

static bool lookup_file(const struct dwarf_line_info* info, uint64_t index, const char** name, uint64_t* dir_index) {
    if (index == 0) return false;

    uint8_t* p = info->file_table;
    uint8_t* end = info->instr_start;
    uint64_t discard;
    for (uint64_t i = 1; i < index; i++) {
        if (!skip_string(&p, end)) return false;
        if (!dwarf_read_uleb(&p, end, &discard)) return false;
        if (!dwarf_read_uleb(&p, end, &discard)) return false;
        if (!dwarf_read_uleb(&p, end, &discard)) return false;
    }

    *name = (const char*)p;
    if (!skip_string(&p, end)) return false;
    if (!dwarf_read_uleb(&p, end, dir_index)) return false;
    return true;
}

static const char* lookup_dir(const struct dwarf_line_info* info, uint64_t index) {
    // index 0 is the compilation directory and isn't stored in the table
    // and we do not wish to print it regardless
    if (index == 0) return nullptr;

    uint8_t* p = info->dir_table;
    uint8_t* end = info->file_table;

    for (uint64_t i = 1; i < index; i++) {
        if (!skip_string(&p, end)) return nullptr;
    }

    if (p >= end || *p == '\0') return nullptr;

    size_t str_len = strnlen((const char*)p, (size_t)(end - p));
    if (p + str_len >= end) return nullptr;

    return (const char*)p;
}

static bool resolve_row(const struct dwarf_line_info* info, const struct line_vm* vm, struct dwarf_line_result* out) {
    const char* name = nullptr;
    uint64_t dir_index = 0;
    if (!lookup_file(info, vm->cand_file, &name, &dir_index))
        return false;

    out->file = name;
    out->line = vm->cand_line;
    out->dir  = lookup_dir(info, dir_index);

    return true;
}

static void dwarf_line_parse_v3(uint8_t* unit, struct dwarf_line_info* out) {
    const struct dwarf_line_header_v3* header = (const struct dwarf_line_header_v3*)unit;
    out->max_ops = 1;
    out->min_inst_len = header->min_inst_len;
    out->line_base = header->line_base;
    out->line_range = header->line_range;
    out->opcode_base = header->opcode_base;
}

static void dwarf_line_parse_v4(uint8_t* unit, struct dwarf_line_info* out) {
    const struct dwarf_line_header_v4* header = (const struct dwarf_line_header_v4*)unit;
    out->min_inst_len = header->min_inst_len;
    out->max_ops = header->max_ops;
    out->line_base = header->line_base;
    out->line_range = header->line_range;
    out->opcode_base = header->opcode_base;
}

static bool dwarf_line_parse_tables(uint8_t* unit, uint8_t* end, struct dwarf_line_info* out) {
    memset(out, 0, sizeof(struct dwarf_line_info));
    if ((uint64_t)(end - unit) < sizeof(struct dwarf_line_header_v4)) return false;

    struct dwarf_line_header_common* common = (void*)unit;
    uint32_t unit_length = common->unit_length;
    uint16_t version = common->version;
    uint32_t header_length = common->header_length;

    if (version != 3 && version != 4) return false;

    uint64_t header_size = version == 3 ? sizeof(struct dwarf_line_header_v3)
                                        : sizeof(struct dwarf_line_header_v4);

    // DWARF64 isn't handled
    if (unit_length == UINT32_MAX) return false;
    if (unit_length > (end - unit) - 4) return false;

    uint8_t* unit_end = unit + 4 + unit_length;
    if (unit_length < header_size - 4) return false;
    if (header_length > (uint64_t)(unit_end - (unit + 10))) return false;

    out->instr_start = unit + 10 + header_length;
    out->instr_end = unit_end;

    if (version == 3) dwarf_line_parse_v3(unit, out);
    else              dwarf_line_parse_v4(unit, out);

    if (out->line_range == 0)  return false;
    if (out->opcode_base == 0) return false;
    if (out->max_ops == 0)     return false;

    uint8_t* instr_start = out->instr_start;
    if (header_length < (header_size - 10) + out->opcode_base - 1) return false;
    uint8_t* p = unit + header_size + out->opcode_base - 1;

    out->dir_table = p;

    // walk over the dir table
    while (p < instr_start) {
        if (*p == '\0') { p++; break; }
        if (!skip_string(&p, instr_start)) return false;
    }

    out->file_table = p;
    return true;
}

static bool dwarf_line_find(const struct dwarf_line_info* info, uint64_t pc, struct dwarf_line_result* out) {
    uint8_t* p = info->instr_start;
    uint8_t* end = info->instr_end;
    struct line_vm vm = {
        .line = 1,
        .file = 1,
    };

    while (p < end) {
        uint8_t opcode = *p++;
        uint64_t operandA;
        int64_t  operandB;

        if (opcode >= info->opcode_base) {
            uint64_t adjusted = opcode - info->opcode_base;
            uint64_t op_advance = adjusted / info->line_range;

            vm.address += info->min_inst_len * (op_advance / info->max_ops);
            vm.line += (int64_t)info->line_base + (adjusted % info->line_range);

            if (emit_row(&vm, pc) && vm.cand_file != 0)
                return resolve_row(info, &vm, out);
            continue;
        }

        switch (opcode) {
            case 0x00: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                if (operandA == 0 || operandA > (uint64_t)(end - p)) return false;

                uint8_t* next = p + operandA;
                uint8_t sub = *p++;

                switch (sub) {
                    case DW_LNE_end_sequence: {
                        if (pc < vm.address && vm.cand_file != 0)
                            return resolve_row(info, &vm, out);
                        vm.cand_file = 0;
                        vm.address = 0;
                        vm.line = 1;
                        vm.file = 1;
                        break;
                    }
                    case DW_LNE_set_address: {
                        uint8_t addr_size = operandA - 1;
                        if (addr_size == 0 || addr_size > sizeof(vm.address)) return false;
                        if (!dwarf_read_bytes(&p, next, addr_size, &vm.address)) return false;
                        break;
                    }
                    case DW_LNE_set_discriminator: {
                        if (!dwarf_read_uleb(&p, next, &operandA)) return false;
                        break;
                    }
                    // opcode is not emitted by modern toolchains and is annoying to handle
                    // just abort if we encounter this
                    case DW_LNE_define_file: return false;
                    default: return false;
                }

                p = next;
                break;
            }

            // emit a row without advancing state
            case DW_LNS_copy: {
                if (emit_row(&vm, pc) && vm.cand_file != 0)
                    return resolve_row(info, &vm, out);
                break;
            }

            // opcodes that directly set vm states
            case DW_LNS_advance_pc: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                vm.address += info->min_inst_len * (operandA / info->max_ops);
                break;
            }
            case DW_LNS_advance_line: {
                if (!dwarf_read_sleb(&p, end, &operandB)) return false;
                vm.line += operandB;
                break;
            }
            case DW_LNS_set_file: {
                if (!dwarf_read_uleb(&p, end, &vm.file)) return false;
                break;
            }

            // opcodes for advancing the PC
            case DW_LNS_const_add_pc: {
                operandA = (255 - info->opcode_base) / info->line_range;
                vm.address += info->min_inst_len * (operandA / info->max_ops);
                break;
            }
            case DW_LNS_fixed_advance_pc: {
                if (!dwarf_read_bytes(&p, end, sizeof(uint16_t), &operandA)) return false;
                vm.address += operandA;
                break;
            }

            // opcodes with one uleb operand and no info we care about
            case DW_LNS_set_column:
            case DW_LNS_set_isa: {
                if (!dwarf_read_uleb(&p, end, &operandA)) return false;
                break;
            }
            // opcodes with no operands and no info we care about
            case DW_LNS_negate_stmt:
            case DW_LNS_set_basic_block:
            case DW_LNS_set_prologue_end:
            case DW_LNS_set_epilogue_begin:
                break;
            default: return false;
        }
    }

    return false;
}

bool dwarf_line_for_pc(void* section, uint64_t size, uint64_t pc, struct dwarf_line_result* out) {
    if (section == nullptr || size < 4) return false;

    uint8_t* unit = section;
    uint8_t* end = unit + size;

    while ((end - unit) >= 4) {
        struct dwarf_line_header_common* common = (struct dwarf_line_header_common*)unit;
        if (common->unit_length == UINT32_MAX) return false;
        if (common->unit_length > (uint64_t)(end - unit) - 4) return false;

        struct dwarf_line_info info;
        if (dwarf_line_parse_tables(unit, end, &info) && dwarf_line_find(&info, pc, out))
            return true;

        unit += 4 + common->unit_length;
    }

    return false;
}
