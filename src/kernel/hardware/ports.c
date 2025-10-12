#include <stdint.h>

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ( "inb %1, %0"
    : "=a"(ret)
    : "Nd"(port) );
    return ret;
}
