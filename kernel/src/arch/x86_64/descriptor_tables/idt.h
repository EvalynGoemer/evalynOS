#pragma once
#include <stdint.h>

extern void setup_bsp_idt();
extern void setup_ap_idt();

extern int32_t rel_isr_table[256];
