#include "../hardware/ports.h"

void spurious_isr(__attribute__((unused)) void* frame) {
    outb(0x20,0x20);
    outb(0xA0,0x20);
}
