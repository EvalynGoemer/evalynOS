#pragma once
#include <stdint.h>

#define INTERRUPT_HANDLER_DOUBLE_FAULT 0x08
#define INTERRUPT_HANDLER_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_HANDLER_PAGE_FAULT 0x0E

#define INTERRUPT_HANDLER_PIT 0x20
#define INTERRUPT_HANDLER_PS2 0x21
#define INTERRUPT_HANDLER_SERIAL 0x24

#define INTERRUPT_HANDLER_SPURIOUS_PIC_1 0x27
#define INTERRUPT_HANDLER_SPURIOUS_PIC_2 0x2F

#define INTERRUPT_HANDLER_SYSCALL 0x69

extern void isr0x08();
extern void isr0x0D();
extern void isr0x0E();

extern void isr0x20();
extern void isr0x21();
extern void isr0x24();

extern void isr0x27();
extern void isr0x2F();

extern void isr0x69();

// Reserved Exception (Used as placeholder for generic)
extern void isr0x16();

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector, error;
    uint64_t ip, cs, flags, rsp, ss;
};

extern void dispatch_interupt (struct interrupt_frame *frame);

#include <interupts/double_fault.h>
#include <interupts/gp_fault.h>
#include <interupts/page_fault.h>
#include <interupts/pit.h>
#include <interupts/ps2.h>
#include <interupts/spurious.h>
#include <interupts/syscall.h>
#include <interupts/generic.h>
#include <interupts/serial.h>
