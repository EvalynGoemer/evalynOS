#include <arch/generic/panic.h>
#include <arch/generic/cpu/halt.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <stdio.h>
#include <inttypes.h>

[[noreturn]]
void panic_interrupt(char* message, irq_saved_regs_t* regs, irq_cpu_frame_t* frame, [[maybe_unused]] uint64_t vector) {
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
    printf("RAX=0x%016lx RBX=0x%016lx ", regs->rax, regs->rbx);
    printf("RCX=0x%016lx RDX=0x%016lx\n", regs->rcx, regs->rdx);
    printf("RSI=0x%016lx RDI=0x%016lx ", regs->rsi, regs->rdi);
    printf("RBP=0x%016lx RSP=0x%016lx\n", regs->rbp, frame->rsp);
    printf("R8 =0x%016lx R9 =0x%016lx ", regs->r8, regs->r9);
    printf("R10=0x%016lx R11=0x%016lx\n", regs->r10, regs->r11);
    printf("R12=0x%016lx R13=0x%016lx ", regs->r12, regs->r13);
    printf("R14=0x%016lx R15=0x%016lx\n", regs->r14, regs->r15);

    printf("\033[38;2;231;133;255mInterrupt Frame:\n");
    printf("IP=0x%016lx SP=0x%016lx\n", frame->ip, frame->rsp);
    printf("SS=0x%016lx CS=0x%016lx\n", frame->ss, frame->cs);
    printf("FLAGS: %08b %08b %08b %08b\n",
           (int)frame->flags >> 24 & 0xFF,
           (int)frame->flags >> 16 & 0xFF,
           (int)frame->flags >> 8  & 0xFF,
           (int)frame->flags       & 0xFF);
    printf("ERROR: %08b %08b %08b %08b\n",
           (int)frame->error >> 24 & 0xFF,
           (int)frame->error >> 16 & 0xFF,
           (int)frame->error >> 8  & 0xFF,
           (int)frame->error       & 0xFF);

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
