#include <stdint.h>
#include "font8x8_basic.h"

uint64_t frameBufferBase;
uint32_t pixelsPerScanLine;

void plotPixel(int x, int y, int color) {
    *((uint32_t*)(frameBufferBase + 4 * pixelsPerScanLine * y + 4 * x)) = color;
}

void printChar(char c, int x, int y) {
    int index = c;
    for (int row = 0; row < 8; row++) {
        unsigned char bits = font8x8_basic[index][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                plotPixel(x + col, y + row, 0xFFFFFFFF);
            } else {
                plotPixel(x + col, y + row, 0x00000000);
            }
        }
    }
}

void printString(const char* str, int x, int y) {
    while (*str) {
        printChar(*str++, x, y);
        x += 8;
    }
}

struct __attribute__((packed)) GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
};

struct __attribute__((packed)) GDTR {
    uint16_t limit;
    uint64_t base;
};

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

static struct GDTEntry gdt[3];
static struct GDTR gdtr;

static struct IDTEntry idt[256];
static struct IDTR idtr;

int interuptsTriggered = 0;

__attribute__((interrupt))
void default_isr(void* frame) {
    interuptsTriggered++;

    char result[2048];
    int num = interuptsTriggered;

    char *p = result;
    const char *base = "interuptsTriggered: ";
    while (*base) *p++ = *base++;

    char temp[256];
    int i = 0;
    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
    }

    while (i > 0) {
        *p++ = temp[--i];
    }
    *p = '\0';


    printString(result, 10, 50);
}

void setup_gdt() {
    gdt[0] = (struct GDTEntry){ 0 }; // Null

    // Code Segment: base=0, limit=0xFFFFF, access=0x9A, gran=0x20
    gdt[1] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A,             // Present, ring 0, code, executable, readable
        .granularity = 0x20,        // Long mode
        .base_high = 0
    };

    // Data Segment: access=0x92, gran=0x00
    gdt[2] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92,             // Present, ring 0, data, writable
        .granularity = 0x00,
        .base_high = 0
    };

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    asm volatile ("lgdt %0" : : "m"(gdtr));
    asm volatile (
        "mov $0x10, %%ax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%ss \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "pushq $0x08 \n"
        "lea 1f(%%rip), %%rax \n"
        "pushq %%rax \n"
        "lretq \n"
        "1:\n"
        : : : "rax"
    );
}

void set_idt_entry(int index, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;

    idt[index] = (struct IDTEntry){
        .offset_low = addr & 0xFFFF,
        .selector = 0x08,
        .ist = 0,
        .type_attr = 0x8E, // Interrupt gate, ring 0, present
        .offset_mid = (addr >> 16) & 0xFFFF,
        .offset_high = (addr >> 32),
        .zero = 0
    };
}

void setup_idt() {
    for (int i = 0; i < 256; ++i)
        set_idt_entry(i, (void (*)())default_isr);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
}

void kernel_main(uint64_t fbb, uint32_t pps) __attribute__((section(".entry")));
void kernel_main(uint64_t fbb, uint32_t pps) {
    frameBufferBase = fbb;
    pixelsPerScanLine = pps;

    printString("Kernel: Kernel Started", 10, 10);

    setup_gdt();
    setup_idt();

    printString("Kernel: Basic GDT & IDT Setup", 10, 10 + 8);

    printString("Kernel: Starting Infinite Divide by Zero to test ISR", 10, 10 + 16);

    while (1) {
        volatile int a = 1;
        volatile int b = 0;
        volatile int c = a / b; // Divide by Zero to trigger ISR
    }
}
