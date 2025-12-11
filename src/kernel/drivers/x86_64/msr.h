#pragma once
#include <stdint.h>

enum MSRs : uint32_t {
    // Extended Feature Enable Register
    EFER   = 0xC0000080,

    // SYSCALL Related MSRs
    STAR   = 0xC0000081,
    LSTAR  = 0xC0000082,
    CSTAR  = 0xC0000083,
    SFMASK = 0xC0000084,

    // GS BASE
    UGSBAS = 0xC0000101,
    KGSBAS = 0xC0000102,
};

extern uint64_t rdmsr(uint32_t msr) ;
extern void wrmsr(uint32_t msr, uint64_t value);
