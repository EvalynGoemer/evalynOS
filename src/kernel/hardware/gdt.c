#include <stdint.h>

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

static struct GDTEntry gdt[3];
static struct GDTR gdtr;

void setup_gdt() {
    gdt[0] = (struct GDTEntry){ 0 };

    gdt[1] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A,
        .granularity = 0x20,
        .base_high = 0
    };

    gdt[2] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92,
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
