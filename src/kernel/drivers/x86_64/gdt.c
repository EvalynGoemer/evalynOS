#include <drivers/x86_64/gdt.h>
#include <stdint.h>

__attribute__ ((aligned (16))) uint8_t kernel_stack[4096];

__attribute__ ((aligned (16))) uint8_t df_stack[4096];

__attribute__ ((aligned (16))) uint8_t nmi_stack[4096];

static union GDTEntry gdt[8];
static struct GDTR gdtr;
struct TSS tss;

void gdt_fill_entry (int num, uint8_t access, uint8_t flags) {
    gdt[num].gdt_entry.limit_low = 0;
    gdt[num].gdt_entry.base_low = 0;
    gdt[num].gdt_entry.base_mid = 0;
    gdt[num].gdt_entry.access = access;
    gdt[num].gdt_entry.limit_high = 0;
    gdt[num].gdt_entry.flags = flags;
    gdt[num].gdt_entry.base_high = 0;
}

void gdt_set_tss(int num) {
    uint64_t tss_base = (uint64_t)&tss;
    gdt[num].gdt_entry.limit_low = sizeof(tss) - 1;
    gdt[num].gdt_entry.base_low = tss_base & 0xffff;
    gdt[num].gdt_entry.base_mid = (tss_base >> 16) & 0xff;
    gdt[num].gdt_entry.access = 0x89;
    gdt[num].gdt_entry.limit_high = ((sizeof(tss) - 1) >> 16) & 0x0F;
    gdt[num].gdt_entry.flags = 0;
    gdt[num].gdt_entry.base_high = (tss_base >> 24) & 0xff;
    gdt[num + 1].tss_addr.tss_addr = tss_base >> 32;
}


void setup_gdt() {
    gdt_fill_entry (0, 0, 0);      // Null                | 0x00
    gdt_fill_entry (1, 0x9A, 0xA); // 64 Bit Kernel Code  | 0x08
    gdt_fill_entry (2, 0x92, 0xC); // 64 Bit Kernel Data  | 0x10
    gdt_fill_entry (3, 0xFA, 0xC); // 32 Bit User Code    | 0x18
    gdt_fill_entry (4, 0xF2, 0xC); // 64 Bit User Data    | 0x20
    gdt_fill_entry (5, 0xFA, 0xA); // 64 Bit User Code    | 0x28
    gdt_set_tss (6);               // TSS Entry 1/2       | 0x30
                                   // TSS Entry 2/2       | 0x38

    tss.ist[1] = (uint64_t)(df_stack + sizeof (df_stack));
    tss.ist[2] = (uint64_t)(nmi_stack + sizeof (nmi_stack));
    tss.rsp0 = (uint64_t)(kernel_stack + sizeof (kernel_stack));

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    lgdt(&gdtr);
    reloadSegments();
    ltr(0x30);
}
