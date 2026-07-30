#include <arch/loongarch64/cpu/interrupts.h>
#include <arch/loongarch64/timer/timer.h>
#include <sched/scheduler.h>
#include <arch/generic/panic.h>
#include <stdint.h>
#include <arch/loongarch64/intrin/csr.h>
#include <stdio.h>
#include <utils/lib.h>

const char *exception_names[] = {
    "Interrupt (0x00)",
    "Page Invalid For Load (0x01)",
    "Page Invalid For Store (0x02)",
    "Page Invalid For Fetch (0x03)",
    "Page Non-Writable (0x04)",
    "Page Non-Readable (0x05)",
    "Page Non-Executable (0x06)",
    "Page Privilege Level Illegal (0x07)",
    "Address Exception (0x08)",
    "Address Alignment Exception (0x09)",
    "Bound Check (0x0A)",
    "System Call (0x0B)",
    "Breakpoint (0x0C)",
    "Invalid Instruction (0x0D)",
    "Instruction Privilege Error (0x0E)",
    "Floating-Point Disable (0x0F)",
    "128-Bit SIMD Disable (0x10)",
    "256-Bit ASIMD Disable (0x11)",
    "Floating-Point Error (0x12)",
    "Watchpoint Exception (0x13)",
    "Binary Translation Disabled Exception (0x14)",
    "Binary Translation Exception (0x15)",
    "Guest Sensitive Privileged Resource (0x16)",
    "Hypervisor Call (0x17)",
    "Guest CSR Change Exception (0x18)",
};

void dispatch_interrupt(interrupt_frame_t* frame) {
    uint8_t  ecode   = (frame->estat >> 16) & 0x3F;
    uint16_t irq_num =  frame->estat        & 0x1FFF;

    if (ecode == 0 && !(irq_num & 0x800))
        enable_interrupts();

    bool handled = false;

    switch (ecode) {
        case 0xC: {
            LOG("got break 0 exception");
            frame->era += 4;
            break;
        }
        case 0x0: {
            if (irq_num & 0x800) {
                timer_set_timeout_ms(5);
                enable_interrupts();
                schedule();
                handled = true;
            }

            if (!handled) {
                LOG_TAGGED("FATAL", ANSI_RED, "Got Unhandled IRQ 0x%x", irq_num);
                panic_interrupt("Unhandled IRQ", frame);
            }
            break;
        }
        default: {
            if (ecode < ARRAY_SIZE(exception_names))
                panic_interrupt(exception_names[ecode], frame);
            else
                panic_interrupt("Reserved exception", frame);
            break;
        }
    }
}

void setup_interrupts() {
    csrwr(CSR_ECFG, 0);
    csrwr(CSR_EENTRY, (uint64_t)interrupt_handler_asm);
}
