#pragma once

#include <stdint.h>

// raw dwarf packets
struct [[gnu::packed]] dwarf_entry_header {
    uint32_t length;
    uint32_t id;
};

struct [[gnu::packed]] dwarf_cie_packet {
    struct dwarf_entry_header header;
    uint8_t version;
    uint8_t data[];
};

// decoded packets
struct dwarf_cie {
    bool     valid;
    bool     is_signal_frame;
    uint64_t offset;
    uint64_t code_align;
    int64_t  data_align;
    uint64_t ra_col;
    uint8_t  addr_size;
    uint8_t  seg_size;
    uint8_t* opcodes_start;
    uint8_t* opcodes_end;
};

struct dwarf_fde {
    uint64_t initial_location;
    uint8_t* opcodes_start;
    uint8_t* opcodes_end;
};

extern bool dwarf_get_fde(void* section, uint64_t section_length, uint64_t pc, struct dwarf_fde* out, struct dwarf_cie* cie);
