#pragma once
#include <stdint.h>

static inline uint64_t read_cr0() {
    uint64_t val;
    asm volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline uint64_t read_cr2() {
    uint64_t val;
    asm volatile ("mov %%cr2, %0" : "=r"(val));
    return val;
}

static inline uint64_t read_cr3() {
    uint64_t val;
    asm volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline uint64_t read_cr4() {
    uint64_t val;
    asm volatile ("mov %%cr4, %0" : "=r"(val));
    return val;
}

static inline uint64_t read_cr8() {
    uint64_t val;
    asm volatile ("mov %%cr8, %0" : "=r"(val));
    return val;
}

static inline void write_cr0(uint64_t val) {
    asm volatile("mov %0, %%cr0" :: "r"(val) : "memory");
}

static inline void write_cr2(uint64_t val) {
    asm volatile("mov %0, %%cr2" :: "r"(val) : "memory");
}

static inline void write_cr3(uint64_t val) {
    asm volatile("mov %0, %%cr3" :: "r"(val) : "memory");
}

static inline void write_cr4(uint64_t val) {
    asm volatile("mov %0, %%cr4" :: "r"(val) : "memory");
}

static inline void write_cr8(uint64_t val) {
    asm volatile("mov %0, %%cr8" :: "r"(val) : "memory");
}

#define CR0_BIT_MP 1
#define CR0_BIT_EM 2
#define CR0_BIT_TS 3
#define CR0_BIT_NE 5

static inline void set_cr0_bit(unsigned int bit) {
    uint64_t val = read_cr0();
    val |= (1ull << bit);
    write_cr0(val);
}

static inline void clear_cr0_bit(unsigned int bit) {
    uint64_t val = read_cr0();
    val &= ~(1ull << bit);
    write_cr0(val);
}

#define CR4_BIT_PSE         4
#define CR4_BIT_MCE         6
#define CR4_BIT_OSFXSR      9
#define CR4_BIT_OSXMMEXCPT  10
#define CR4_BIT_SMEP        20
#define CR4_BIT_SMAP        21
#define CR4_BIT_FRED        32

static inline void set_cr4_bit(unsigned int bit) {
    uint64_t val = read_cr4();
    val |= (1ull << bit);
    write_cr4(val);
}

static inline void clear_cr4_bit(unsigned int bit) {
    uint64_t val = read_cr4();
    val &= ~(1ull << bit);
    write_cr4(val);
}

extern void setup_control_regs();
extern bool smep_enabled;
extern bool smap_enabled;
