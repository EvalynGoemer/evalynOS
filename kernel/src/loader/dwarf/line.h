#pragma once

#include <stdint.h>

struct [[gnu::packed]] dwarf_line_header_common {
    uint32_t unit_length;
    uint16_t version;
    uint32_t header_length;
};

struct [[gnu::packed]] dwarf_line_header_v3 {
    uint32_t unit_length;
    uint16_t version;
    uint32_t header_length;
    uint8_t  min_inst_len;
    uint8_t  default_is_stmt;
    int8_t   line_base;
    uint8_t  line_range;
    uint8_t  opcode_base;
};

struct [[gnu::packed]] dwarf_line_header_v4 {
    uint32_t unit_length;
    uint16_t version;
    uint32_t header_length;
    uint8_t  min_inst_len;
    uint8_t  max_ops;
    uint8_t  default_is_stmt;
    int8_t   line_base;
    uint8_t  line_range;
    uint8_t  opcode_base;
};

struct dwarf_line_info {
    uint8_t  min_inst_len;
    uint8_t  max_ops;
    int8_t   line_base;
    uint8_t  line_range;
    uint8_t  opcode_base;
    uint8_t* dir_table;
    uint8_t* file_table;
    uint8_t* instr_start;
    uint8_t* instr_end;
};

struct dwarf_line_result {
    const char* dir;
    const char* file;
    uint64_t    line;
};

extern bool dwarf_line_for_pc(void* section, uint64_t size, uint64_t pc, struct dwarf_line_result* out);
