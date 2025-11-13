#include <stdint.h>
#include <drivers/x86_64/ports.h>

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void play_sound(uint16_t dx) {
    outb(0x43, 0xB6);
    outb(0x42, dx & 0xFF);
    outb(0x42, (dx >> 8) & 0xFF);
    uint8_t al = inb(0x61);
    al |= 0x03;
    outb(0x61, al);
}

void stop_sound() {
    uint8_t al = inb(0x61);
    al &= 0xFC;
    outb(0x61, al);
}
