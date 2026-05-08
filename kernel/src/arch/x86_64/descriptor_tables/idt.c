#include <stdint.h>
#include <stdio.h>

#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/cpu/interrupts.h>

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

static struct IDTEntry bsp_idt[256];
static struct IDTR bsp_idtr;

void set_idt_entry(struct IDTEntry idt[], int index, int ist, int attr, uint64_t addr) {
    idt[index] = (struct IDTEntry){
        .offset_low = addr & 0xFFFF,
        .selector = 0x08,
        .ist = ist,
        .type_attr = attr,
        .offset_mid = (addr >> 16) & 0xFFFF,
        .offset_high = (addr >> 32),
        .zero = 0
    };
}

static inline uint64_t resolve_isr(int vector) {
    return (uintptr_t)&rel_isr_table[vector] + rel_isr_table[vector];
}

void setup_bsp_idt() {
    for (int i = 0x00; i < 256; ++i) {
        set_idt_entry(bsp_idt, i, 0, 0x8E, resolve_isr(i));
    }

    set_idt_entry(bsp_idt, INTERRUPT_DOUBLE_FAULT,            1, 0x8E, resolve_isr(INTERRUPT_DOUBLE_FAULT));
    set_idt_entry(bsp_idt, INTERRUPT_NON_MASKABLE_INTERRUPT,  2, 0x8E, resolve_isr(INTERRUPT_NON_MASKABLE_INTERRUPT));
    set_idt_entry(bsp_idt, INTERRUPT_MACHINE_CHECK_EXCEPTION, 3, 0x8E, resolve_isr(INTERRUPT_MACHINE_CHECK_EXCEPTION));

    bsp_idtr.limit = sizeof(bsp_idt) - 1;
    bsp_idtr.base = (uint64_t)&bsp_idt;

    asm volatile ("lidt %0" : : "m"(bsp_idtr));

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "IDT INIT")
}
