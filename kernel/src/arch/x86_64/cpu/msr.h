#pragma once
#include <stdint.h>

// LAPIC
#define MSR_APIC_BASE           0x0000001B

// Machine Check Exception Handling
#define MSR_MCE_CAP             0x00000179
#define MSR_MCE_STATUS          0x0000017A
#define MSR_MCE_CTL             0x0000017B
#define MSR_MCEn_CTL(n)         (0x00000400 + ((n) * 4))
#define MSR_MCEn_STATUS(n)      (0x00000401 + ((n) * 4))
#define MSR_MCEn_ADDR(n)        (0x00000402 + ((n) * 4))
#define MSR_MCEn_MISC(n)        (0x00000403 + ((n) * 4))

// FRED (Flexible Return and Event Delivery)
#define MSR_FRED_CONFIG         0x000001D4
#define MSR_FRED_RSP0           0x000001CC
#define MSR_FRED_RSP1           0x000001CD
#define MSR_FRED_RSP2           0x000001CE
#define MSR_FRED_RSP3           0x000001CF
#define MSR_FRED_STKLVLS        0x000001D0

// x2APIC
#define MSR_x2APIC_ID           0x00000802
#define MSR_x2APIC_EOI          0x0000080B
#define MSR_x2APIC_SVR          0x0000080F

// x2APIC Timer
#define MSR_x2APIC_TIMER        0x00000832
#define MSR_x2APIC_TIMER_LINT0  0x00000835
#define MSR_x2APIC_TIMER_LINT1  0x00000836
#define MSR_x2APIC_TIMER_ICOUNT 0x00000838
#define MSR_x2APIC_TIMER_CCOUNT 0x00000839
#define MSR_x2APIC_TIMER_DIVIDE 0x0000083E

// TSC
#define MSR_TSC                 0x00000010
#define MSR_TSC_DEADLINE        0x000006E0

// Extended Feature Enable Register
#define MSR_EFER                0xC0000080

// SYSCALL Related MSRs
#define MSR_STAR                0xC0000081
#define MSR_LSTAR               0xC0000082
#define MSR_CSTAR               0xC0000083
#define MSR_SFMASK              0xC0000084

// Segment Register Bases
#define MSR_FSBASE              0xC0000100
#define MSR_UGSBASE             0xC0000101
#define MSR_KGSBASE             0xC0000102

static inline uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return low + ((uint64_t) high << 32);
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr) : "memory");
}
