#pragma once
#include <stdint.h>

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ( "inb %w1, %b0"
    : "=a"(ret)
    : "Nd"(port)
    : "memory");
    return ret;
}

static inline void io_wait() {
    outb(0x80, 0);
}

static inline void outbd(unsigned short port, unsigned char val) {
    io_wait();
    outb(port, val);
    io_wait();
}

static inline unsigned char inbd(unsigned short port) {
    io_wait();
    unsigned char ret = inb(port);
    io_wait();
    return ret;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile (
        "inl %1, %0"
        : "=a"(value)
        : "dN"(port)
        : "memory");
    return value;
}
