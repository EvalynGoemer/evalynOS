#include "./interupts.h"
#include "../panic.h"

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void double_fault_isr(struct interrupt_frame* frame, uint64_t error) {
    panic("Double Fault", 0x08, frame, error, PANIC_FLAGS_FRAME | PANIC_FLAGS_ERROR | PANIC_FLAGS_VECTOR);
}
