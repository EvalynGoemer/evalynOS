#include <stdint.h>

#include <interupts/interupts.h>
#include <utils/panic.h>

__attribute__((interrupt))
void gp_fault_isr(struct interrupt_frame* frame, uint64_t error) {
    panic("General Protection Fault", 0x0D, frame, error, PANIC_FLAGS_FRAME | PANIC_FLAGS_ERROR | PANIC_FLAGS_VECTOR);
}
