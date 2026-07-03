#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct [[gnu::packed]] per_ap_data {
    uint64_t sp;
    uint64_t cpulocal_base;
} per_ap_data_t;

typedef struct [[gnu::packed]] global_ap_data {
    uint32_t internal_core_id_counter; // atomic
    uint32_t total_cores;
    uint8_t _padding[56];
    per_ap_data_t* per_ap_data_ptrs[];
} global_ap_data_t;

// static asserts for ap.asm
_Static_assert(offsetof(global_ap_data_t, internal_core_id_counter) == 0);
_Static_assert(offsetof(global_ap_data_t, per_ap_data_ptrs) == 64);

extern uint8_t  x86_ap_trampoline[];
extern uint64_t x86_ap_trampoline_dataptr;
extern uint32_t x86_ap_trampoline_config;
extern uint32_t x86_ap_trampoline_cr3;

extern uint32_t x86_ap_trampoline_gdtr_base;
extern uint32_t x86_ap_trampoline_farjmp;
extern uint32_t x86_ap_trampoline_farjmp64;
extern uint64_t x86_ap_trampoline_gdt[];
extern uint8_t  x86_ap_trampoline32[];
extern uint8_t  x86_ap_trampoline64[];

#define x86_AP_TRAMPOLINE_CONF_NX   (1 << 0)
#define x86_AP_TRAMPOLINE_CONF_LA57 (1 << 1)
