#pragma once

extern volatile int pitInteruptsTriggered;
extern volatile int shouldSchedule;

__attribute__((interrupt))
extern void pit_isr(void* frame);
