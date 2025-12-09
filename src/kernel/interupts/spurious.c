#include <drivers/x86_64/ports.h>

void spurious_isr() {
    outb(0x20,0x20);
    outb(0xA0,0x20);
}
