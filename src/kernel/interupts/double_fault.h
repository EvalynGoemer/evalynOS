#include <stdint.h>

__attribute__((interrupt))
extern void double_fault_isr(struct interrupt_frame* frame, uint64_t error);
