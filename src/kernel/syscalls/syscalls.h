#pragma once
#include <stdint.h>

struct syscall_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
};

extern void init_syscall();
extern void syscall_handler();
extern void execute_syscall(struct syscall_frame* frame);
