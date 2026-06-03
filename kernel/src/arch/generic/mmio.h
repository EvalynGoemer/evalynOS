#pragma once
#include <stdint.h>

#if defined(__x86_64__)
static inline void mmio_write_64(uint64_t addr, uint64_t value) {
    asm volatile ("sfence" ::: "memory");
    asm volatile ("movq %[val], (%[addr])" : : [addr] "r" (addr), [val] "a" (value) : "memory");
    asm volatile ("sfence" ::: "memory");
}
static inline void mmio_write_32(uint64_t addr, uint32_t value) {
    asm volatile ("sfence" ::: "memory");
    asm volatile ("movl %[val], (%[addr])" : : [addr] "r" (addr), [val] "a" (value) : "memory");
    asm volatile ("sfence" ::: "memory");
}
static inline void mmio_write_16(uint64_t addr, uint16_t value) {
    asm volatile ("sfence" ::: "memory");
    asm volatile ("movw %[val], (%[addr])" : : [addr] "r" (addr), [val] "a" (value) : "memory");
    asm volatile ("sfence" ::: "memory");
}
static inline void mmio_write_8(uint64_t addr, uint8_t value) {
    asm volatile ("sfence" ::: "memory");
    asm volatile ("movb %[val], (%[addr])" : : [addr] "r" (addr), [val] "a" (value) : "memory");
    asm volatile ("sfence" ::: "memory");
}

static inline uint64_t mmio_read_64(uint64_t addr) {
    uint64_t r;
    asm volatile ("lfence" ::: "memory");
    asm volatile ("movq (%[addr]), %[val]" : [val] "=a" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("lfence" ::: "memory");
    return r;
}
static inline uint32_t mmio_read_32(uint64_t addr) {
    uint32_t r;
    asm volatile ("lfence" ::: "memory");
    asm volatile ("movl (%[addr]), %[val]" : [val] "=a" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("lfence" ::: "memory");
    return r;
}
static inline uint16_t mmio_read_16(uint64_t addr) {
    uint16_t r;
    asm volatile ("lfence" ::: "memory");
    asm volatile ("movw (%[addr]), %[val]" : [val] "=a" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("lfence" ::: "memory");
    return r;
}
static inline uint8_t mmio_read_8(uint64_t addr) {
    uint8_t r;
    asm volatile ("lfence" ::: "memory");
    asm volatile ("movb (%[addr]), %[val]" : [val] "=a" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("lfence" ::: "memory");
    return r;
}

#elif defined(__loongarch64)
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
#else
_Static_assert(0, "unimplemented");
#endif

static inline void mmio_write_offset_64(uint64_t base_addr, uint64_t offset, uint64_t value) { mmio_write_64(base_addr + offset, value); }
static inline uint64_t mmio_read_offset_64(uint64_t base_addr, uint64_t offset) { return mmio_read_64(base_addr + offset); }
static inline void mmio_write_offset_32(uint64_t base_addr, uint64_t offset, uint32_t value) { mmio_write_32(base_addr + offset, value); }
static inline uint32_t mmio_read_offset_32(uint64_t base_addr, uint64_t offset) { return mmio_read_32(base_addr + offset); }
static inline void mmio_write_offset_16(uint64_t base_addr, uint64_t offset, uint16_t value) { mmio_write_16(base_addr + offset, value); }
static inline uint16_t mmio_read_offset_16(uint64_t base_addr, uint64_t offset) { return mmio_read_16(base_addr + offset); }
static inline void mmio_write_offset_8(uint64_t base_addr, uint64_t offset, uint8_t value) { mmio_write_8(base_addr + offset, value); }
static inline uint8_t mmio_read_offset_8(uint64_t base_addr, uint64_t offset) { return mmio_read_8(base_addr + offset); }
