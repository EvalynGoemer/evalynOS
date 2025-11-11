#include <stddef.h>

#include "interupts.h"
#include "../libc/stdio.h"
#include "../renderer/fb_renderer.h"
#include "../hardware/ports.h"
#include "../scheduler/scheduler.h"

volatile int pitInteruptsTriggered = 0;
volatile int shouldSchedule = 0;
#define LINE_WIDTH 80

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void pit_isr(__attribute__((unused)) void* frame) {
    pitInteruptsTriggered++;

    if (shouldSchedule) {
        schedule();
    }

    outb(0x20,0x20);
}
