#pragma once
#include <stdint.h>

struct [[gnu::packed]] GDTR {
    uint16_t limit;
    uint64_t base;
};

struct [[gnu::packed]] TSS {
    uint32_t _reserved0;
    uint64_t rsp[3];
    uint64_t _reserved1;
    uint64_t ist[7];
    uint8_t  _reserved2[10];
    uint16_t io_map_base;
};

extern void lgdt(struct GDTR* gdtr);
extern void ltr(uint16_t ltr);
extern void reloadSegments();

extern void setup_bsp_gdt();
extern void setup_ap_gdt(uint64_t cpulocal_base);

[[gnu::aligned(64)]] extern uint8_t bsp_df_stack[4096];
[[gnu::aligned(64)]] extern uint8_t bsp_nmi_stack[4096];
[[gnu::aligned(64)]] extern uint8_t bsp_mce_stack[4096];

extern struct TSS tss;
