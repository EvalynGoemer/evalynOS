#pragma once

__attribute__((interrupt))
extern void generic_isr(struct interrupt_frame* frame);
