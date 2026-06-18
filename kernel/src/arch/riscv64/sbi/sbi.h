#pragma once

typedef struct sbiret {
    long error;
    long value;
} sbiret_t;

static inline sbiret_t sbi_ecall(long ext, long fid, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) {
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    register long a3 asm("a3") = arg3;
    register long a4 asm("a4") = arg4;
    register long a5 asm("a5") = arg5;
    register long a6 asm("a6") = fid;
    register long a7 asm("a7") = ext;
    asm volatile ("ecall"
        : "+r"(a0), "+r"(a1)
        : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
        : "memory");
    return (sbiret_t){.error = a0, .value = a1};
}

static inline sbiret_t sbi_ecall0(long ext, long fid) { return sbi_ecall(ext, fid, 0, 0, 0, 0, 0, 0); }
static inline sbiret_t sbi_ecall1(long ext, long fid, long a0) { return sbi_ecall(ext, fid, a0, 0, 0, 0, 0, 0); }
static inline sbiret_t sbi_ecall2(long ext, long fid, long a0, long a1) { return sbi_ecall(ext, fid, a0, a1, 0, 0, 0, 0); }
static inline sbiret_t sbi_ecall3(long ext, long fid, long a0, long a1, long a2) { return sbi_ecall(ext, fid, a0, a1, a2, 0, 0, 0); }
static inline sbiret_t sbi_ecall4(long ext, long fid, long a0, long a1, long a2, long a3) { return sbi_ecall(ext, fid, a0, a1, a2, a3, 0, 0); }
static inline sbiret_t sbi_ecall5(long ext, long fid, long a0, long a1, long a2, long a3, long a4) { return sbi_ecall(ext, fid, a0, a1, a2, a3, a4, 0); }
static inline sbiret_t sbi_ecall6(long ext, long fid, long a0, long a1, long a2, long a3, long a4, long a5) { return sbi_ecall(ext, fid, a0, a1, a2, a3, a4, a5); }
