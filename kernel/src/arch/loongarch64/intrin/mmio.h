#pragma once
#include <stdint.h>

static inline uint64_t mmio_read_64(uint64_t addr) {
    uint64_t r;
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("ld.d %[val], %[addr], 0" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("dbar 0" ::: "memory");
    return r;
}
static inline uint32_t mmio_read_32(uint64_t addr) {
    uint32_t r;
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("ld.wu %[val], %[addr], 0" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("dbar 0" ::: "memory");
    return r;
}
static inline uint16_t mmio_read_16(uint64_t addr) {
    uint16_t r;
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("ld.hu %[val], %[addr], 0" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("dbar 0" ::: "memory");
    return r;
}
static inline uint8_t mmio_read_8(uint64_t addr) {
    uint8_t r;
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("ld.bu %[val], %[addr], 0" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("dbar 0" ::: "memory");
    return r;
}

static inline void mmio_write_64(uint64_t addr, uint64_t value) {
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("st.d %[val], %[addr], 0" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("dbar 0" ::: "memory");
}
static inline void mmio_write_32(uint64_t addr, uint32_t value) {
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("st.w %[val], %[addr], 0" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("dbar 0" ::: "memory");
}

static inline void mmio_write_16(uint64_t addr, uint16_t value) {
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("st.h %[val], %[addr], 0" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("dbar 0" ::: "memory");
}

static inline void mmio_write_8(uint64_t addr, uint8_t value) {
    asm volatile ("dbar 0" ::: "memory");
    asm volatile ("st.b %[val], %[addr], 0" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("dbar 0" ::: "memory");
}
