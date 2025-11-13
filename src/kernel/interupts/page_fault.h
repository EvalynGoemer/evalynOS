#pragma once
#include <stdint.h>

__attribute__((interrupt))
extern void page_fault_isr(struct interrupt_frame* frame, uint64_t error);
