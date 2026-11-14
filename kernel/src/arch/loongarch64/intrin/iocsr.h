#pragma once

#define IOCSR_IPI_SEND      0x1040
#define IOCSR_IPI_MAIL_SEND 0x1048

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>

inline void iocsr_write_8(size_t addr, uint8_t value) {
    asm volatile ("iocsrwr.b %0, %1" :: "r"(value), "r"(addr) : "memory");
}

inline void iocsr_write_16(size_t addr, uint8_t value) {
    asm volatile ("iocsrwr.h %0, %1" :: "r"(value), "r"(addr) : "memory");
}

inline void iocsr_write_32(size_t addr, uint32_t value) {
    asm volatile ("iocsrwr.w %0, %1" :: "r"(value), "r"(addr) : "memory");
}

inline void iocsr_write_64(size_t addr, uint64_t value) {
    asm volatile ("iocsrwr.d %0, %1" :: "r"(value), "r"(addr) : "memory");
}

inline uint8_t iocsr_read_8(size_t addr) {
    uint8_t value;
    asm volatile ("iocsrrd.b %0, %1" : "=r"(value) : "r"(addr));
    return value;
}

inline uint16_t iocsr_read_16(size_t addr) {
    uint16_t value;
    asm volatile ("iocsrrd.h %0, %1" : "=r"(value) : "r"(addr));
    return value;
}

inline uint32_t iocsr_read_32(size_t addr) {
    uint32_t value;
    asm volatile ("iocsrrd.w %0, %1" : "=r"(value) : "r"(addr));
    return value;
}

inline uint64_t iocsr_read_64(size_t addr) {
    uint64_t value;
    asm volatile ("iocsrrd.d %0, %1" : "=r"(value) : "r"(addr));
    return value;
}

#endif
