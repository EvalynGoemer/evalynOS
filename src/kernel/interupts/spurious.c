#include "../hardware/ports.h"
#include "generic.h"

void spurious_isr(__attribute__((unused)) void* frame) {
    genericInteruptsTriggered++;
    outb(0x20,0x20);
    outb(0xA0,0x20);
}
