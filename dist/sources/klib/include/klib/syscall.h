#pragma once
#include <stddef.h>

#ifdef __x86_64__
#define SYSCALL_INST       "syscall"
#define SYSCALL_RETVAL_REG "rax"
#define SYSCALL_ERRVAL_REG "rdx"
#define SYSCALL_SYSNUM_REG "rdi"
#define SYSCALL_ARG1_REG   "rsi"
#define SYSCALL_ARG2_REG   "rdx"
#define SYSCALL_ARG3_REG   "rax" // sysv is rcx but but syscall clobbers it so use rax
#define SYSCALL_ARG4_REG   "r8"
#define SYSCALL_ARG5_REG   "r9"
#define SYSCALL_ARG6_REG   "r10" // sysv has no 7th argument register so use r10
#define SYSCALL_CLOBBERS   "rcx", "r11",
#elif defined(__riscv)
#define SYSCALL_INST       "ecall"
#define SYSCALL_RETVAL_REG "a0"
#define SYSCALL_ERRVAL_REG "a1"
#define SYSCALL_SYSNUM_REG "a0"
#define SYSCALL_ARG1_REG   "a1"
#define SYSCALL_ARG2_REG   "a2"
#define SYSCALL_ARG3_REG   "a3"
#define SYSCALL_ARG4_REG   "a4"
#define SYSCALL_ARG5_REG   "a5"
#define SYSCALL_ARG6_REG   "a6"
#define SYSCALL_CLOBBERS
#elif defined(__loongarch64)
#define SYSCALL_INST       "syscall 0"
#define SYSCALL_RETVAL_REG "a0"
#define SYSCALL_ERRVAL_REG "a1"
#define SYSCALL_SYSNUM_REG "a0"
#define SYSCALL_ARG1_REG   "a1"
#define SYSCALL_ARG2_REG   "a2"
#define SYSCALL_ARG3_REG   "a3"
#define SYSCALL_ARG4_REG   "a4"
#define SYSCALL_ARG5_REG   "a5"
#define SYSCALL_ARG6_REG   "a6"
#define SYSCALL_CLOBBERS
#endif

#define SYS_MISC_CTL          0
#define SYS_ALLOC_ANON        1
#define SYS_FREE_ANON         2
#define SYS_EXIT              3

typedef struct {
    size_t retval;
    size_t error;
} sysret_t;

static inline sysret_t syscall0(size_t syscall_num) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}

static inline sysret_t syscall1(size_t syscall_num, size_t arg1) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}

static inline sysret_t syscall2(size_t syscall_num, size_t arg1, size_t arg2) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    register size_t a2  asm(SYSCALL_ARG2_REG)   = arg2;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1), "r"(a2)
        : SYSCALL_CLOBBERS "memory"
    );

    return (sysret_t){ret, err};
}

static inline sysret_t syscall3(size_t syscall_num, size_t arg1, size_t arg2, size_t arg3) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    register size_t a2  asm(SYSCALL_ARG2_REG)   = arg2;
    register size_t a3  asm(SYSCALL_ARG3_REG)   = arg3;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1), "r"(a2), "r"(a3)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}

static inline sysret_t syscall4(size_t syscall_num, size_t arg1, size_t arg2, size_t arg3, size_t arg4) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    register size_t a2  asm(SYSCALL_ARG2_REG)   = arg2;
    register size_t a3  asm(SYSCALL_ARG3_REG)   = arg3;
    register size_t a4  asm(SYSCALL_ARG4_REG)   = arg4;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}

static inline sysret_t syscall5(size_t syscall_num, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    register size_t a2  asm(SYSCALL_ARG2_REG)   = arg2;
    register size_t a3  asm(SYSCALL_ARG3_REG)   = arg3;
    register size_t a4  asm(SYSCALL_ARG4_REG)   = arg4;
    register size_t a5  asm(SYSCALL_ARG5_REG)   = arg5;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}

static inline sysret_t syscall6(size_t syscall_num, size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t arg5, size_t arg6) {
    register size_t ret asm(SYSCALL_RETVAL_REG);
    register size_t err asm(SYSCALL_ERRVAL_REG);
    register size_t num asm(SYSCALL_SYSNUM_REG) = syscall_num;
    register size_t a1  asm(SYSCALL_ARG1_REG)   = arg1;
    register size_t a2  asm(SYSCALL_ARG2_REG)   = arg2;
    register size_t a3  asm(SYSCALL_ARG3_REG)   = arg3;
    register size_t a4  asm(SYSCALL_ARG4_REG)   = arg4;
    register size_t a5  asm(SYSCALL_ARG5_REG)   = arg5;
    register size_t a6  asm(SYSCALL_ARG6_REG)   = arg6;
    asm volatile (
        SYSCALL_INST
        : "=r"(ret), "=r"(err)
        : "r"(num), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
        : SYSCALL_CLOBBERS "memory"
    );
    return (sysret_t){ret, err};
}
