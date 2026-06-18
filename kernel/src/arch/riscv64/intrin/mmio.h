#pragma once
#include <stdint.h>

static inline uint64_t mmio_read_64(uint64_t addr) {
    uint64_t r;
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("ld %[val], 0(%[addr])" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
    return r;
}
static inline uint32_t mmio_read_32(uint64_t addr) {
    uint32_t r;
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("lwu %[val], 0(%[addr])" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
    return r;
}
static inline uint16_t mmio_read_16(uint64_t addr) {
    uint16_t r;
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("lhu %[val], 0(%[addr])" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
    return r;
}
static inline uint8_t mmio_read_8(uint64_t addr) {
    uint8_t r;
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("lbu %[val], 0(%[addr])" : [val] "=r" (r) : [addr] "r" (addr) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
    return r;
}

static inline void mmio_write_64(uint64_t addr, uint64_t value) {
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("sd %[val], 0(%[addr])" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
}
static inline void mmio_write_32(uint64_t addr, uint32_t value) {
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("sw %[val], 0(%[addr])" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
}
static inline void mmio_write_16(uint64_t addr, uint16_t value) {
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("sh %[val], 0(%[addr])" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
}
static inline void mmio_write_8(uint64_t addr, uint8_t value) {
    asm volatile ("fence iorw, iorw" ::: "memory");
    asm volatile ("sb %[val], 0(%[addr])" :: [addr] "r" (addr), [val] "r" (value) : "memory");
    asm volatile ("fence iorw, iorw" ::: "memory");
}
