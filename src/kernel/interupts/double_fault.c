#include <interupts/interupts.h>
#include <utils/panic.h>

__attribute__((interrupt))
void double_fault_isr(struct interrupt_frame* frame, uint64_t error) {
    panic("Double Fault", 0x08, frame, error, PANIC_FLAGS_FRAME | PANIC_FLAGS_ERROR | PANIC_FLAGS_VECTOR);
}
