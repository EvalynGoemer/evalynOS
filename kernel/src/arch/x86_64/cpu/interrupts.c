#include "arch/x86_64/intrin/interrupts.h"
#include <arch/generic/panic.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <arch/x86_64/drivers/fred/fred.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/timer/timer.h>
#include <sched/scheduler.h>
#include <arch/intrin/interrupts.h>
#include <stdint.h>
#include <stdio.h>

void dispatch_interrupt(interrupt_frame_t* frame) {
    uint64_t vector;

    /* IDT Logic */
    if (!fred_enabled) {
        vector = frame->vector;
        goto skip_fred;
    }

    /* FRED Logic */
    vector = (frame->cpu_frame.ss >> 32) & 0xFF;
    if (frame->vector == FRED_FAKE_VECTOR_CPL3) {
        uint8_t type = (frame->cpu_frame.ss >> 48) & 0xF;
        if (type == FRED_EVENT_TYPE_SYSCALL) {
            // TODO: when syscalls are setup replace this with the handle syscall function
            panic("Kernel should not be getting syscalls right now");
            return;
        }
        // TODO: when ring3 is properly setup remove this
        panic("Kernel should not be getting interrupts from ring3 right now");
    }
    skip_fred:

    /* Main Interrupt path */
    switch (vector) {
        case 0xFA:
            LOG("got test vector 0xFA");
            break;
        case INTERRUPT_MACHINE_CHECK_EXCEPTION:
            handle_exception_mce(frame);
            break;
        case LAPIC_TIMER_VECTOR:
            // make sure to call this every timer irq to ensure
            // rollover is handled when the HPET or ACPI PMT is in use
            timer_get_ns();

            timer_set_timeout_ms(1);
            arch_send_eoi();
            enable_interrupts();
            schedule();
            break;
        default: {
            if (vector <= 0x1F) {
                panic_interrupt(exception_names[vector], frame);
            } else {
                LOG_TAGGED("FATAL", ANSI_RED, "Got Unhandled IRQ 0x%02lx", vector)
                panic_interrupt("\x1b[1A", frame);
            }
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
