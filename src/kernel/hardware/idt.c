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

void set_idt_entry(int index, int ist, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;

    idt[index] = (struct IDTEntry){
        .offset_low = addr & 0xFFFF,
        .selector = 0x08,
        .ist = ist,
        .type_attr = 0x8E,
        .offset_mid = (addr >> 16) & 0xFFFF,
        .offset_high = (addr >> 32),
        .zero = 0
    };
}

void setup_idt() {
    for (int i = 0x1F; i < 256; ++i)
        set_idt_entry(i, 0, (void (*)())generic_isr);

    set_idt_entry(INTERRUPT_HANDLER_DOUBLE_FAULT, 1, (void (*)())double_fault_isr);
    set_idt_entry(INTERRUPT_HANDLER_GENERAL_PROTECTION_FAULT, 0, (void (*)())gp_fault_isr);
    set_idt_entry(INTERRUPT_HANDLER_PAGE_FAULT, 0, (void (*)())page_fault_isr);
    set_idt_entry(INTERRUPT_HANDLER_PIT, 0, (void (*)())pit_isr);
    set_idt_entry(INTERRUPT_HANDLER_PS2, 0, (void (*)())ps2_isr);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
}
