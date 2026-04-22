#include "utils/locks/spinlock.h"
#include <arch/generic/panic.h>
#include <arch/generic/cpu/halt.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <stdio.h>
#include <inttypes.h>

[[noreturn]]
void panic_interrupt(const char* message, interrupt_frame_t* frame) {
    disable_interrupts();

    if (__atomic_exchange_n(&panic_flag, 1, __ATOMIC_SEQ_CST) != 0)
        hcf();

    // force unlock stdio for panic; keep IRQs disabled
    spinlock_unlock(&stdio_spinlock, 0);

    panic_print_start(message);

    unsigned long cr0, cr2, cr3, cr4, cr8;
    asm volatile(
        "mov %%cr0, %0\n\t"
        "mov %%cr2, %1\n\t"
        "mov %%cr3, %2\n\t"
        "mov %%cr4, %3\n\t"
        "mov %%cr8, %4\n\t"
        : "=r"(cr0), "=r"(cr2), "=r"(cr3), "=r"(cr4), "=r"(cr8)
    );

    printf("\033[38;2;175;56;255mGeneral Registers:\n");
    printf("RAX=0x%016lx RBX=0x%016lx ", frame->regs.rax, frame->regs.rbx);
    printf("RCX=0x%016lx RDX=0x%016lx\n", frame->regs.rcx, frame->regs.rdx);
    printf("RSI=0x%016lx RDI=0x%016lx ", frame->regs.rsi, frame->regs.rdi);
    printf("RBP=0x%016lx RSP=0x%016lx\n", frame->regs.rbp, frame->cpu_frame.rsp);
    printf("R8 =0x%016lx R9 =0x%016lx ", frame->regs.r8, frame->regs.r9);
    printf("R10=0x%016lx R11=0x%016lx\n", frame->regs.r10, frame->regs.r11);
    printf("R12=0x%016lx R13=0x%016lx ", frame->regs.r12, frame->regs.r13);
    printf("R14=0x%016lx R15=0x%016lx\n", frame->regs.r14, frame->regs.r15);

    printf("\033[38;2;231;133;255mInterrupt Frame:\n");
    printf("IP=0x%016lx SP=0x%016lx\n", frame->cpu_frame.ip, frame->cpu_frame.rsp);
    printf("SS=0x%016lx CS=0x%016lx\n", frame->cpu_frame.ss, frame->cpu_frame.cs);
    printf("FLAGS: %08b %08b %08b %08b\n",
           (int)frame->cpu_frame.flags >> 24 & 0xFF,
           (int)frame->cpu_frame.flags >> 16 & 0xFF,
           (int)frame->cpu_frame.flags >> 8  & 0xFF,
           (int)frame->cpu_frame.flags       & 0xFF);
    printf("ERROR: %08b %08b %08b %08b\n",
           (int)frame->cpu_frame.error >> 24 & 0xFF,
           (int)frame->cpu_frame.error >> 16 & 0xFF,
           (int)frame->cpu_frame.error >> 8  & 0xFF,
           (int)frame->cpu_frame.error       & 0xFF);

    printf("\033[38;2;255;238;0mControl Registers:\n");
    printf("CR0: %08b %08b %08b %08b\n",
           (int)cr0 >> 24 & 0xFF,
           (int)cr0 >> 16 & 0xFF,
           (int)cr0 >> 8  & 0xFF,
           (int)cr0       & 0xFF);
    printf("CR4: %08b %08b %08b %08b\n",
           (int)cr4 >> 24 & 0xFF,
           (int)cr4 >> 16 & 0xFF,
           (int)cr4 >> 8  & 0xFF,
           (int)cr4       & 0xFF);
    printf("CR8: %08b %08b %08b %08b\n",
           (int)cr8 >> 24 & 0xFF,
           (int)cr8 >> 16 & 0xFF,
           (int)cr8 >> 8  & 0xFF,
           (int)cr8       & 0xFF);
    printf("CR2=0x%016lx CR3=0x%016lx\n", cr2, cr3);

    panic_print_end();

    hcf();
}
