#include "stdint.h"

#include "./interupts.h"
#include "../panic.h"
#include "../libc/stdio.h"
#include "../renderer/fb_renderer.h"

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void page_fault_isr(struct interrupt_frame* frame, uint64_t error) {
    panic("Page Fault", 0x0E, frame, error, PANIC_FLAGS_FRAME | PANIC_FLAGS_ERROR | PANIC_FLAGS_VECTOR);
}
