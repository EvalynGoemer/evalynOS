#include <arch/x86_64/panic.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <stdio.h>

void dispatch_interrupt(irq_saved_regs_t* regs, irq_cpu_frame_t* frame, uint64_t vector) {
    switch (vector) {
        case 0xFA:
            printf("got test vector 0xFA\n");
            break;
        default:
            if (vector <= 0x1F) {
                panic_interrupt((char*)exception_names[vector], regs, frame, vector);
            } else {
                printf(ANSI_RED "[FATAL] Got Unhandled IRQ 0x%02lx\n", vector);
                panic_interrupt("\x1b[1A", regs, frame, vector);
            }
    }
}

const char *exception_names[] = {
    "#DE (0x00) - Division Error",
    "#DB (0x01) - Debug Exception",
    "#NMI (0x02) - Non Maskable Interrupt",
    "#BP (0x03) - Breakpoint Exception",
    "#OF (0x04) - Overflow Exception",
    "#BR (0x05) - Bound Range Exception",
    "#UD (0x06) - Invalid Opcode",
    "#NM (0x07) - FPU Not Found",
    "#DF (0x08) - Double Fault",
    "#CSO (0x09) - Coprocessor Segment Overrun",
    "#TS (0x0A) - Invalid TSS",
    "#NP (0x0B) - Missing Segment",
    "#SS (0x0C) - Invalid Stack Segment",
    "#GPF (0x0D) - General Protection Fault",
    "#PF (0x0E) - Page Fault",
    "??? (0x0F) - Reserved",
    "#MF (0x10) - x87 FPU Exception",
    "#AC (0x11) - Alignment Check Exception",
    "#MCE (0x12) - Machine Check Exception",
    "#XM (0x13) - SIMD FPU Exception",
    "#VE (0x14) - Virtualization Exception",
    "#CP (0x15) - Control Protection Exception",
    "??? (0x16) - Reserved",
    "??? (0x17) - Reserved",
    "??? (0x18) - Reserved",
    "??? (0x19) - Reserved",
    "??? (0x1A) - Reserved",
    "??? (0x1B) - Reserved",
    "#HV (0x1C) - Hypervisor Injection Exception",
    "#HC (0x1D) - VMM Communication Exception",
    "#DX (0x1E) - Security Exception",
    "??? (0x1F) - Reserved",
};
