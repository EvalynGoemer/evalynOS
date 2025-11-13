#include <interupts/interupts.h>
#include <utils/panic.h>

__attribute__((interrupt))
void generic_isr(struct interrupt_frame* frame) {
    panic("Unhandled interupt", 0, frame, 0, PANIC_FLAGS_FRAME);
}
