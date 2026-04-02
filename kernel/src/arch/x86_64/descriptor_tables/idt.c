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

void set_idt_entry(struct IDTEntry idt[], int index, int ist, int attr, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;

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

void setup_bsp_idt() {
    for (int i = 0x00; i < 256; ++i) {
        // set the ist here because UBSAN triggers otherwise
        int ist = 0;
        if (i == INTERRUPT_DOUBLE_FAULT) ist = 1;
        if (i == INTERRUPT_NON_MASKABLE_INTERRUPT) ist = 2;
        set_idt_entry(bsp_idt, i, ist, 0x8E, isr_table[i]);
    }

    bsp_idtr.limit = sizeof(bsp_idt) - 1;
    bsp_idtr.base = (uint64_t)&bsp_idt;

    asm volatile ("lidt %0" : : "m"(bsp_idtr));

    printf(ANSI_BYELLOW "[ARCH EARLY INIT]" ANSI_RESET " IDT INIT " ANSI_BGREEN "[OK]\n" ANSI_RESET);
}
