#include <stdint.h>
#include <stdio.h>

#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/cpu/interrupts.h>

struct [[gnu::packed]] IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

struct [[gnu::packed]] IDTR {
    uint16_t limit;
    uint64_t base;
};

static struct IDTEntry idt[256] = {0};

void set_idt_entry(int index, int ist, int attr, uint64_t addr) {
    idt[index] = (struct IDTEntry) {
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
        int ist = 0;
        if (i == INTERRUPT_DOUBLE_FAULT)            ist = 1;
        if (i == INTERRUPT_NON_MASKABLE_INTERRUPT)  ist = 2;
        if (i == INTERRUPT_MACHINE_CHECK_EXCEPTION) ist = 3;
        set_idt_entry(i, ist, 0x8E, resolve_isr(i));
    }


    struct IDTR idtr = {.limit = sizeof(idt) - 1, .base = (uint64_t)idt};
    asm volatile ("lidt %0" : : "m"(idtr));

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "IDT INIT")
}

void setup_ap_idt() {
    struct IDTR idtr = {.limit = sizeof(idt) - 1, .base = (uint64_t)idt};
    asm volatile ("lidt %0" : : "m"(idtr));
}
