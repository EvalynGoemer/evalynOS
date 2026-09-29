#include "arch/intrin/cpulocal.h"
#include <arch/x86_64/cpu/msr.h>
#include <utils/lib.h>
#include <stdio.h>
#include <stdint.h>
#include <mem/pmm.h>
#include <arch/x86_64/descriptor_tables/gdt.h>

[[gnu::aligned(64)]] uint8_t bsp_df_stack[4096];
[[gnu::aligned(64)]] uint8_t bsp_nmi_stack[4096];
[[gnu::aligned(64)]] uint8_t bsp_mce_stack[4096];

CPU_LOCAL uint64_t gdt[8]  = {
    0x0000000000000000, // Null                | 0x00
    0x00af9b000000ffff, // 64 Bit Kernel Code  | 0x08
    0x00af93000000ffff, // 64 Bit Kernel Data  | 0x10
    0x00cffa000000ffff, // 32 Bit User Code    | 0x18
    0x00aff3000000ffff, // 64 Bit User Data    | 0x20
    0x00affb000000ffff, // 64 Bit User Code    | 0x28
    0x0000000000000000, // TSS Entry 1/2       | 0x30
    0x0000000000000000, // TSS Entry 2/2       | 0x38
};

CPU_LOCAL struct TSS tss = {0};

static void set_tss(uint64_t gdt[8], struct TSS* tss) {
    uint64_t tss_base = (uint64_t)tss;

    gdt[6] =  (sizeof(struct TSS) - 1) & 0xFFFF; // limit low
    gdt[6] |= (tss_base & 0xFFFF) << 16;         // base low
    gdt[6] |= ((tss_base >> 16) & 0xFF) << 32;   // base mid
    gdt[6] |= ((tss_base >> 24) & 0xFF) << 56;   // base high
    gdt[6] |= (0x89ull) << 40;                   // access

    // limit high and flags are zero

    gdt[7] = tss_base >> 32;
}

void setup_bsp_gdt() {
    uint64_t* local_gdt = (void*)CPU_LOCAL_PTR(gdt);
    struct TSS* local_tss = (void*)CPU_LOCAL_PTR(tss);

    set_tss(local_gdt, local_tss);

    local_tss->ist[0] = (uint64_t)(bsp_df_stack + sizeof (bsp_df_stack));
    local_tss->ist[1] = (uint64_t)(bsp_nmi_stack + sizeof (bsp_nmi_stack));
    local_tss->ist[2] = (uint64_t)(bsp_mce_stack + sizeof (bsp_mce_stack));
    local_tss->io_map_base = sizeof(struct TSS);

    struct GDTR gdtr = {.limit = sizeof(gdt) - 1, .base = (uint64_t)local_gdt};

    lgdt(&gdtr);
    reloadSegments();
    ltr(0x30);

    wrmsr(MSR_UGSBASE, 0);
    wrmsr(MSR_KGSBASE, 0);

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "GDT INIT")
}

void setup_ap_gdt(uint64_t cpulocal_base) {
    uint64_t* local_gdt = (void*)CPU_LOCAL_PTR(gdt);
    struct TSS* local_tss = (void*)CPU_LOCAL_PTR(tss);

    set_tss(local_gdt, local_tss);

    local_tss->ist[0] = TO_HHDM(pmm_alloc_page() + PAGE_SIZE);
    local_tss->ist[1] = TO_HHDM(pmm_alloc_page() + PAGE_SIZE);
    local_tss->ist[2] = TO_HHDM(pmm_alloc_page() + PAGE_SIZE);
    local_tss->io_map_base = sizeof(struct TSS);

    struct GDTR gdtr = {.limit = sizeof(gdt) - 1, .base = (uint64_t)local_gdt};

    lgdt(&gdtr);
    reloadSegments();
    ltr(0x30);

    SET_CPU_LOCAL(cpulocal_base);
}
