#include <interupts/interupts.h>
#include <drivers/x86_64/ports.h>
#include <scheduler/scheduler.h>

volatile int pitInteruptsTriggered = 0;
volatile int shouldSchedule = 0;
#define LINE_WIDTH 80

__attribute__((interrupt))
void pit_isr(__attribute__((unused)) void* frame) {
    pitInteruptsTriggered++;

    if (shouldSchedule) {
        schedule();
    }

    outb(0x20,0x20);
}
