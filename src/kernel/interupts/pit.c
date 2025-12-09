#include <interupts/interupts.h>
#include <drivers/x86_64/ports.h>
#include <scheduler/scheduler.h>

volatile int pitInteruptsTriggered = 0;
volatile int shouldSchedule = 0;

void pit_isr() {
    pitInteruptsTriggered++;

    if (shouldSchedule) {
        schedule();
    }

    outb(0x20,0x20);
}
