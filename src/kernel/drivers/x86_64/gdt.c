#include <stdint.h>

__attribute__ ((aligned (16))) uint8_t kernel_stack[16384];

__attribute__ ((aligned (16))) uint8_t user_stack[65536];

__attribute__ ((aligned (16))) uint8_t df_stack[65536];

__attribute__ ((aligned (16))) uint8_t nmi_stack[65536];


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

struct __attribute__ ((packed)) TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3, io_map_base;
};


static struct GDTEntry gdt[7];
static struct GDTR gdtr;
static struct TSS tss;

void gdt_fill_entry (int num, uint8_t access, uint8_t granularity, uint32_t base, uint32_t limit) {
    gdt[num].limit_low = limit & 0xFFFF;
    gdt[num].base_low = base & 0xFFFF;
    gdt[num].base_mid = (base >> 16) & 0xFF;
    gdt[num].access = access;
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    gdt[num].base_high = (base >> 24) & 0xFF;
}

void gdt_set_tss (int num, uint64_t base, uint32_t limit) {
    gdt_fill_entry (num, 0x89, 0x00, base, limit);
    uint32_t *hi = (uint32_t *)&gdt[num + 1];
    hi[0] = base >> 32;
    hi[1] = 0;
}

void setup_gdt() {
    gdt_fill_entry (0, 0, 0, 0, 0);
    gdt_fill_entry (1, 0x9A, 0x20, 0, 0);
    gdt_fill_entry (2, 0x92, 0x00, 0, 0);
    gdt_set_tss (3, (uint64_t)&tss, sizeof (tss) - 1);
    gdt_fill_entry (5, 0xFA, 0x20, 0, 0);
    gdt_fill_entry (6, 0xF2, 0x00, 0, 0);

    tss.ist[0] = (uint64_t)(df_stack + sizeof (df_stack));
    tss.ist[1] = (uint64_t)(nmi_stack + sizeof (nmi_stack));
    tss.rsp0 = (uint64_t)(kernel_stack + sizeof (kernel_stack));

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

    asm volatile ("ltr %%ax" ::"a"(0x18));
}
