#pragma once
#include <stdint.h>
#include <string.h>

static inline bool dwarf_read_uleb_impl(uint8_t** p, uint8_t* end, uint64_t* out, int* shift_out) {
    uint64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;

    while (true) {
        // ensure it's in bounds and the shift won't go over
        if (*p >= end) return false;
        if (shift >= 64) return false;

        // get the byte and accumulate it
        byte = *(*p)++;
        result |= (uint64_t)(byte & 0x7f) << shift;
        shift += 7;

        // bit 7 being zero means there is no more data
        if ((byte & 0x80) == 0) break;
    }
    *out = result;
    if (shift_out) *shift_out = shift;
    return true;
}

static inline bool dwarf_read_uleb(uint8_t** p, uint8_t* end, uint64_t* out) {
    return dwarf_read_uleb_impl(p, end, out, nullptr);
}

static inline bool dwarf_read_sleb(uint8_t** p, uint8_t* end, int64_t* out) {
    uint64_t value = 0;
    int shift = 0;

    // get the value as unsigned first
    if (!dwarf_read_uleb_impl(p, end, &value, &shift)) return false;
    int64_t result = value;

    // sign extend if needed
    if (shift < 64 && (result >> (shift - 1)) & 1)
        result |= ~0ULL << shift;

    *out = result;
    return true;
}

static inline bool dwarf_read_bytes(uint8_t** p, uint8_t* end, uint8_t bytes, uint64_t* out) {
    if ((uint64_t)(end - *p) < bytes) return false;
    *out = 0;
    memcpy(out, *p, bytes);
    *p += bytes;
    return true;
}
