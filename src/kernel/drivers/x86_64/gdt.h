#pragma once
#include <stdint.h>

struct __attribute__ ((packed)) TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3, io_map_base;
};

extern void setup_gdt();
extern struct TSS tss;
