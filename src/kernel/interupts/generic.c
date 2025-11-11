#include <stdint.h>

#include "./interupts.h"
#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"
#include "../panic.h"

#define LINE_WIDTH 80
volatile int genericInteruptsTriggered = 0;

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void generic_isr(struct interrupt_frame* frame) {
    genericInteruptsTriggered++;

    if(genericInteruptsTriggered > 0) {
        panic("Too many unhandled interupts", 0, frame, 0, PANIC_FLAGS_FRAME);
    }

    outb(0x20,0x20);
    outb(0xa0,0x20);
}
