#pragma once
#include <stdint.h>

extern void setup_bsp_idt();
extern int32_t rel_isr_table[256];
