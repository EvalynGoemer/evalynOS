#include "stdint.h"

#include "./interupts.h"
#include "../panic.h"

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void gp_fault_isr(struct interrupt_frame* frame, uint64_t error) {
    panic("General Protection Fault", 0x0D, frame, error, PANIC_FLAGS_FRAME | PANIC_FLAGS_ERROR | PANIC_FLAGS_VECTOR);
}
