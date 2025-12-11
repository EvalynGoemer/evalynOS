#pragma once
#include <stdint.h>

struct TSSGDTAddr {
    uint64_t tss_addr;
};

struct __attribute__((packed)) GDTGDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit_high : 4;
    uint8_t  flags : 4;
    uint8_t  base_high;
};

union __attribute__((packed)) GDTEntry {
    struct GDTGDTEntry gdt_entry;
    struct TSSGDTAddr tss_addr;
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

extern void lgdt(struct GDTR* gdtr);
extern void ltr(uint16_t ltr);
extern void reloadSegments();

extern void setup_gdt();
extern struct TSS tss;
