#pragma once
#include <stdint.h>

extern void setup_mmu(uint64_t root, uint64_t handler);
extern void tlb_refill_handler();
