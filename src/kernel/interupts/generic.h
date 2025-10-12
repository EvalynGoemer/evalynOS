#pragma once
extern volatile int genericInteruptsTriggered;
extern void generic_isr(struct interrupt_frame* frame);
