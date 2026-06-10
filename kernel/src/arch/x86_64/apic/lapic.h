#pragma once
#include <stdint.h>

#define APIC_REGISTER_ID     0x020
#define APIC_REGISTER_EOI    0x0B0
#define APIC_REGISTER_SVR    0x0F0
#define APIC_REGISTER_TIMER  0x320
#define APIC_REGISTER_DIVIDE 0x3E0
#define APIC_REGISTER_ICOUNT 0x380
#define APIC_REGISTER_CCOUNT 0x390
#define APIC_REGISTER_ICRL   0x300
#define APIC_REGISTER_ICRH   0x310

#define x2APIC_REGISTER_ICR  0x830

#define LAPIC_BASE_x1ENABLE (1 << 11)
#define LAPIC_BASE_x2ENABLE (1 << 10)

#define LAPIC_ICR_DELIVERY_STATUS (1 << 12)

#define LAPIC_ICR_DMODE_SIPI (0b110 << 8)
#define LAPIC_ICR_DMODE_INIT (0b101 << 8)

extern void setup_lapic();

extern uint32_t arch_get_local_coreid();
extern void arch_send_ipi(uint32_t target_coreid, uint16_t vector);
extern void x86_send_sipi(uint32_t target_coreid, uint8_t starting_page);
extern void x86_send_init(uint32_t target_coreid);
