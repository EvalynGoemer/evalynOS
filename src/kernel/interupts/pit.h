#pragma once
extern volatile int pitInteruptsTriggered;
extern volatile int shouldSchedule;
extern void pit_isr(void* frame);
