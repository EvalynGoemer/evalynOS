#pragma once

extern void setup_bsp_idt();
extern void (*isr_table[256])(void);
