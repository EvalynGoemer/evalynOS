#pragma once
extern volatile int ps2InteruptsTriggered;
extern volatile unsigned char ps2LastScanCode;
extern volatile int ps2IrqFired;

void ps2_isr(void* frame);
