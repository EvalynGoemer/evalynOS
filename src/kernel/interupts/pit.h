#pragma once
extern volatile int pitInteruptsTriggered;
extern void pit_isr(void* frame);
