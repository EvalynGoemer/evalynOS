#include <interupts/interupts.h>
#include <utils/panic.h>

void page_fault_isr(struct interrupt_frame* frame) {
    panic("Page Fault", frame);
}
