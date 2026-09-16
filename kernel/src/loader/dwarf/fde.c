#include <stdint.h>
#include <string.h>
#include <loader/dwarf/fde.h>
#include <loader/dwarf/helpers.h>

static struct dwarf_cie_packet* dwarf_get_cie(void* section, uint64_t section_length, uint64_t cie_pointer) {
    if (cie_pointer + sizeof(struct dwarf_entry_header) > section_length) return nullptr;
    struct dwarf_entry_header* e = section + cie_pointer;
    if (e->id != UINT32_MAX) return nullptr;
    if (cie_pointer + 4 + e->length > section_length) return nullptr;
    return (struct dwarf_cie_packet*)e;
}

static bool dwarf_decode_cie(struct dwarf_cie_packet* cie, struct dwarf_cie* out) {
    if (cie->header.length < 5) return false;

    // TODO: handle version 1 since gcc uses that
    if (cie->version != 4) return false;

    uint8_t* end = (uint8_t*)cie + 4 + cie->header.length;
    uint8_t* p = cie->data;

    // cie's from .debug_frame will have a single 'S' as the augment if its a signal frame
    // TODO: upstream the 0001-fix-debug-frame-augment.patch at some point since it makes
    // llvm have the same behavior as gcc for this as seen here https://godbolt.org/z/3bvxh7Pse

    if (p < end && *p == 'S') {
        out->is_signal_frame = true;
        p++;
    } else {
        out->is_signal_frame = false;
    }

    if (p >= end || *p != '\0') return false;
    p++;

    if (p + 2 > end) return false;
    uint8_t addr_size = *p++;
    uint8_t seg_size = *p++;
    if (addr_size == 0 || addr_size > 8) return false;

    if (!dwarf_read_uleb(&p, end, &out->code_align)) return false;
    if (!dwarf_read_sleb(&p, end, &out->data_align)) return false;
    if (!dwarf_read_uleb(&p, end, &out->ra_col)) return false;

    if (out->code_align == 0) return false;

    out->addr_size = addr_size;
    out->seg_size = seg_size;
    out->opcodes_start = p;
    out->opcodes_end = end;
    out->valid = true;
    return true;
}

bool dwarf_get_fde(void* section, uint64_t section_length, uint64_t pc, struct dwarf_fde* fde_out, struct dwarf_cie* cie_out) {
    uint8_t* end = section + section_length;
    cie_out->valid = false;

    for (uint8_t* p = section; p + sizeof(struct dwarf_entry_header) <= end;) {
        struct dwarf_entry_header* e = (void*)p;

        // DWARF64 isn't handled
        if (e->length == UINT32_MAX) break;
        if (e->length < 4) break;

        if (e->length > (uint64_t)(end - p) - 4) break;
        uint8_t* entry_end = p + 4ull + e->length;

        if (e->id != UINT32_MAX) {
            // use the valid marker and the offset itself to not decode the cie every time
            if (!cie_out->valid || cie_out->offset != e->id) {
                struct dwarf_cie_packet* packet = dwarf_get_cie(section, section_length, e->id);
                if (!packet || !dwarf_decode_cie(packet, cie_out))
                    return false;
                cie_out->offset = e->id;
            }

            // ensure the next fields are in bounds
            if (4 + cie_out->seg_size + 2 * cie_out->addr_size > e->length)
                return false;

            uint8_t* cursor = p + 8;
            if (cie_out->seg_size) cursor += cie_out->seg_size;

            uint64_t initial_location = 0;
            memcpy(&initial_location, cursor, cie_out->addr_size);
            cursor += cie_out->addr_size;

            uint64_t address_range = 0;
            memcpy(&address_range, cursor, cie_out->addr_size);
            cursor += cie_out->addr_size;

            // if this is the fde we wanted return it
            if (initial_location <= pc && initial_location != 0 && pc - initial_location < address_range) {
                fde_out->initial_location = initial_location;
                fde_out->opcodes_start = cursor;
                fde_out->opcodes_end = entry_end;
                return true;
            }
        }

        p = entry_end;
    }

    return false;
}
