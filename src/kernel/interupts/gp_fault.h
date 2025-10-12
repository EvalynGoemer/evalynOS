#include "stdint.h"

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
extern void gp_fault_isr(struct interrupt_frame* frame, uint64_t error);
