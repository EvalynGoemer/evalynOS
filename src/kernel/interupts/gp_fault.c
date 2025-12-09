#include <interupts/interupts.h>
#include <utils/panic.h>

void gp_fault_isr(struct interrupt_frame* frame) {
    panic("General Protection Fault", frame);
}
