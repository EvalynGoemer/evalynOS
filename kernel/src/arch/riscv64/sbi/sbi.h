#pragma once

typedef struct sbiret {
    long error;
    long value;
} sbiret_t;

typedef struct sbifunc {
    long eid;
    long fid;
} sbifunc_t;

#define SBI_SUCCESS                0
#define SBI_ERR_FAILED            -1
#define SBI_ERR_NOT_SUPPORTED     -2
#define SBI_ERR_INVALID_PARAM     -3
#define SBI_ERR_DENIED            -4
#define SBI_ERR_INVALID_ADDRESS   -5
#define SBI_ERR_ALREADY_AVAILABLE -6
#define SBI_ERR_ALREADY_STARTED   -7
#define SBI_ERR_ALREADY_STOPPED   -8

#define LEGACY_SBI_SET_TIMER       0x00
#define LEGACY_SBI_CONSOLE_PUTCHAR 0x01
#define LEGACY_SBI_CONSOLE_GETCHAR 0x02

#define SBI_GET_SPEC_VER  (sbifunc_t){.eid = 0x10, .fid = 0}
#define SBI_PROBE_SBI_EXT (sbifunc_t){.eid = 0x10, .fid = 3}
#define SBI_SET_TIMER     (sbifunc_t){.eid = 0x54494D45, .fid = 0}

static inline sbiret_t sbi_ecall1(sbifunc_t func, long arg0) {
    register long a7 asm("a7") = func.eid;
    register long a6 asm("a6") = func.fid;
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1");
    asm volatile ("ecall"
    : "+r"(a0), "=r"(a1)
    : "r"(a7), "r"(a6)
    : "memory");
    return (sbiret_t){.error = a0, .value = a1};
}

static inline sbiret_t sbi_ecall0(sbifunc_t func) {
    register long a7 asm("a7") = func.eid;
    register long a6 asm("a6") = func.fid;
    register long a0 asm("a0");
    register long a1 asm("a1");
    asm volatile ("ecall"
    : "=r"(a0), "=r"(a1)
    : "r"(a7), "r"(a6)
    : "memory");
    return (sbiret_t){.error = a0, .value = a1};
}

static inline long legacy_sbi_ecall1(long eid, long arg0) {
    register long a7 asm("a7") = eid;
    register long a0 asm("a0") = arg0;
    asm volatile ("ecall"
        : "+r"(a0)
        : "r"(a7)
        : "memory");
    return a0;
}

static inline long legacy_sbi_ecall0(long eid) {
    register long a7 asm("a7") = eid;
    register long a0 asm("a0");
    asm volatile ("ecall"
    : "=r"(a0)
    : "r"(a7)
    : "memory");
    return a0;
}
