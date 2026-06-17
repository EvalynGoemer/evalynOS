#pragma once
#include <stdint.h>

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
