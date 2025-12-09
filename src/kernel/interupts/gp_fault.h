#pragma once
#include <interupts/interupts.h>
#include <stdint.h>

extern void gp_fault_isr(struct interrupt_frame* frame);
