#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h> // IWYU pragma: keep
#include <limits.h> // IWYU pragma: keep

typedef uint16_t fdt16_t;
typedef uint32_t fdt32_t;
typedef uint64_t fdt64_t;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define be2host16(v) __builtin_bswap16((v))
#define be2host32(v) __builtin_bswap32((v))
#define be2host64(v) __builtin_bswap64((v))
#else
#define be2host16(v) (v)
#define be2host32(v) (v)
#define be2host64(v) (v)
#endif

static inline uint16_t fdt16_to_cpu(fdt16_t x) {
    return be2host16(x);
}

static inline fdt16_t cpu_to_fdt16(uint16_t x) {
    return be2host16(x);
}

static inline uint32_t fdt32_to_cpu(fdt32_t x) {
    return be2host32(x);
}

static inline fdt32_t cpu_to_fdt32(uint32_t x) {
    return be2host32(x);
}

static inline uint64_t fdt64_to_cpu(fdt64_t x) {
    return be2host64(x);
}

static inline fdt64_t cpu_to_fdt64(uint64_t x) {
    return be2host64(x);
}

static inline uint32_t fdt_read_u32(const void* ptr) {
    uint32_t value;
    memcpy(&value, ptr, 4);
    return fdt32_to_cpu(value);
}

static inline uint64_t fdt_read_u64(const void* ptr) {
    uint64_t value;
    memcpy(&value, ptr, 8);
    return fdt64_to_cpu(value);
}
