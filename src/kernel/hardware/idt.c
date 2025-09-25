#include <stdint.h>

#include "../interupts/interupts.h"

struct __attribute__((packed)) IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

struct __attribute__((packed)) IDTR {
    uint16_t limit;
    uint64_t base;
};

static struct IDTEntry idt[256];
static struct IDTR idtr;

void set_idt_entry(int index, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;

    idt[index] = (struct IDTEntry){
        .offset_low = addr & 0xFFFF,
        .selector = 0x08,
        .ist = 0,
        .type_attr = 0x8E,
        .offset_mid = (addr >> 16) & 0xFFFF,
        .offset_high = (addr >> 32),
        .zero = 0
    };
}

void setup_idt() {
    for (int i = 0; i < 256; ++i)
        set_idt_entry(i, (void (*)())generic_isr);

    set_idt_entry(0x20, (void (*)())pit_isr);
    set_idt_entry(0x21, (void (*)())ps2_isr);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
}
