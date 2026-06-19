#include <stdint.h>
#include <stdio.h>
#include <utils/lib.h>
#include <arch/generic/panic.h>
#include <arch/riscv64/intrin/csr.h>
#include <arch/riscv64/cpu/interrupts.h>

const char *exception_names[] = {
    "Instruction address misaligned (0x00)",
    "Instruction access fault (0x01)",
    "Illegal instruction (0x02)",
    "Breakpoint (0x03)",
    "Load address misaligned (0x04)",
    "Load access fault (0x05)",
    "Store/AMO address misaligned (0x06)",
    "Store/AMO access fault (0x07)",
    "Environment call from U-mode (0x08)",
    "Environment call from S-mode (0x09)",
    "Reserved (0x0A)",
    "Reserved (0x0B)",
    "Instruction page fault (0x0C)",
    "Load page fault (0x0D)",
    "Reserved (0x0E)",
    "Store/AMO page fault (0x0F)",
    "Reserved (0x10)",
    "Reserved (0x11)",
    "Software check (0x12)",
    "Hardware error (0x13)",
};

static const char* exception_name(uint64_t cause) {
    if (cause < ARRAY_SIZE(exception_names))
        return exception_names[cause];
    if ((cause >= 24 && cause <= 31) || (cause >= 48 && cause <= 63))
        return "Custom use";
    return "Reserved";
}

void dispatch_interrupt(interrupt_frame_t* frame) {
    switch (frame->scause) {
        case 3: {
            LOG("got ebreak exception");
            // TODO; use a safe read here
            uint16_t insn16 = *(volatile uint16_t *)frame->sepc;
            bool is_rvc = (frame->sepc % 2 == 0) && ((insn16 & 0x3) != 0x3);
            if (is_rvc && insn16 == 0x9002) {
                frame->sepc += 2;
                break;
            }
            frame->sepc += 4;
            break;
        }
        default: {
            bool is_interrupt = (frame->scause >> 63) & 1;
            frame->scause &= ~(1ull << 63);
            if (!is_interrupt) {
                panic_interrupt(exception_name(frame->scause), frame);
            }
            else {
                LOG_TAGGED("FATAL", ANSI_RED, "Got Unhandled IRQ 0x%llx", frame->scause);
                panic_interrupt("Unhandled IRQ", frame);
            }
            break;
        }
    }
}

// HACK: cpu local is required for interrupts
// this is a minimal stub for kernel only intterupts
typedef struct cpu_local {
    void* current_thread; // untouched for kernel interrupts
    uint64_t scrarch[3];
} cpu_local_t;

cpu_local_t cpu_local = {0};

void setup_interrupts() {
    csrw(CSR_SSCRATCH, (uint64_t)&cpu_local);
    csrw(CSR_STVEC, (uint64_t)interrupt_handler_asm);
}
