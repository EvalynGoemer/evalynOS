#include <interupts/interupts.h>
#include <utils/panic.h>

void generic_isr(struct interrupt_frame* frame) {
    panic("Unhandled interupt", frame);
}
