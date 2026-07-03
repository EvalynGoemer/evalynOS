#include <arch/x86_64/cpu/msr.h>
#include <stdio.h>
#include <stdint.h>
#include <arch/x86_64/descriptor_tables/gdt.h>

[[gnu::aligned(64)]] uint8_t bsp_df_stack[4096];
[[gnu::aligned(64)]] uint8_t bsp_nmi_stack[4096];
[[gnu::aligned(64)]] uint8_t bsp_mce_stack[4096];

static union GDTEntry bsp_gdt[8];
static struct GDTR    bsp_gdtr;
struct TSS bsp_tss;

void gdt_fill_entry(union GDTEntry gdt[], int num, uint8_t access, uint8_t flags) {
    gdt[num].gdt_entry.limit_low = 0;
    gdt[num].gdt_entry.base_low = 0;
    gdt[num].gdt_entry.base_mid = 0;
    gdt[num].gdt_entry.access = access;
    gdt[num].gdt_entry.limit_high = 0;
    gdt[num].gdt_entry.flags = flags;
    gdt[num].gdt_entry.base_high = 0;
}

void gdt_set_tss(union GDTEntry gdt[], struct TSS *tss, int num) {
    uint64_t tss_base = (uint64_t)tss;
    gdt[num].gdt_entry.limit_low = sizeof(struct TSS) - 1;
    gdt[num].gdt_entry.base_low = tss_base & 0xffff;
    gdt[num].gdt_entry.base_mid = (tss_base >> 16) & 0xff;
    gdt[num].gdt_entry.access = 0x89;
    gdt[num].gdt_entry.limit_high = ((sizeof(struct TSS) - 1) >> 16) & 0x0F;
    gdt[num].gdt_entry.flags = 0;
    gdt[num].gdt_entry.base_high = (tss_base >> 24) & 0xff;
    gdt[num + 1].tss_addr.tss_addr = tss_base >> 32;
}

void setup_bsp_gdt() {
    gdt_fill_entry(bsp_gdt, 0, 0, 0);       // Null                | 0x00
    gdt_fill_entry(bsp_gdt, 1, 0x9A, 0xA);  // 64 Bit Kernel Code  | 0x08
    gdt_fill_entry(bsp_gdt, 2, 0x92, 0xC);  // 64 Bit Kernel Data  | 0x10
    gdt_fill_entry(bsp_gdt, 3, 0xFA, 0xC);  // 32 Bit User Code    | 0x18
    gdt_fill_entry(bsp_gdt, 4, 0xF2, 0xC);  // 64 Bit User Data    | 0x20
    gdt_fill_entry(bsp_gdt, 5, 0xFA, 0xA);  // 64 Bit User Code    | 0x28
    gdt_set_tss(bsp_gdt, &bsp_tss, 6);      // TSS Entry 1/2       | 0x30
                                            // TSS Entry 2/2       | 0x38

    bsp_tss.ist[0] = (uint64_t)(bsp_df_stack + sizeof (bsp_df_stack));
    bsp_tss.ist[1] = (uint64_t)(bsp_nmi_stack + sizeof (bsp_nmi_stack));
    bsp_tss.ist[2] = (uint64_t)(bsp_mce_stack + sizeof (bsp_mce_stack));
    bsp_tss.io_map_base = sizeof(struct TSS);

    bsp_gdtr.limit = sizeof(bsp_gdt) - 1;
    bsp_gdtr.base = (uint64_t)&bsp_gdt;

    lgdt(&bsp_gdtr);
    reloadSegments();
    ltr(0x30);

    wrmsr(MSR_UGSBASE, 0);
    wrmsr(MSR_KGSBASE, 0);

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "GDT INIT")
}
