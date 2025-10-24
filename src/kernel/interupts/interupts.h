#pragma once
#include <stdint.h>

#define INTERRUPT_HANDLER_DOUBLE_FAULT 0x08
#define INTERRUPT_HANDLER_GENERAL_PROTECTION_FAULT 0x0D
#define INTERRUPT_HANDLER_PAGE_FAULT 0x0E

#define INTERRUPT_HANDLER_PIT 0x20
#define INTERRUPT_HANDLER_PS2 0x21

#define INTERRUPT_HANDLER_SPURIOUS_PIC_1 0x27
#define INTERRUPT_HANDLER_SPURIOUS_PIC_2 0x2F

#define INTERRUPT_HANDLER_SYSCALL 0x69

struct interrupt_frame
{
    uintptr_t ip;
    uintptr_t cs;
    uintptr_t flags;
    uintptr_t sp;
    uintptr_t ss;
};

#include "double_fault.h"
#include "gp_fault.h"
#include "page_fault.h"
#include "pit.h"
#include "ps2.h"
#include "spurious.h"
#include "syscall.h"
#include "generic.h"
