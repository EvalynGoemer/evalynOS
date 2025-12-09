#pragma once
#include <stdint.h>
#include <interupts/interupts.h>

extern void page_fault_isr(struct interrupt_frame*);
