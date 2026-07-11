#pragma once
#include <stdint.h>
#include <stddef.h>

#define AP_TRAMPOLINE_CONF_NX   (1 << 0)
#define AP_TRAMPOLINE_CONF_LA57 (1 << 1)

typedef struct [[gnu::packed]] {
    uint16_t gdtr_limit;
    uint32_t gdtr_base;
    uint32_t fjmp32_addr;
    uint16_t fjmp32_seg;
    uint32_t fjmp64_addr;
    uint16_t fjmp64_seg;

    uint8_t _pad[6];

    void*    data_ptr;
    uint32_t cr3;
    uint32_t config;
} trampoline_data_t;

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
_Static_assert(offsetof(trampoline_data_t, gdtr_limit)  ==  0);
_Static_assert(offsetof(trampoline_data_t, gdtr_base)   ==  2);
_Static_assert(offsetof(trampoline_data_t, fjmp32_addr) ==  6);
_Static_assert(offsetof(trampoline_data_t, fjmp32_seg)  == 10);
_Static_assert(offsetof(trampoline_data_t, fjmp64_addr) == 12);
_Static_assert(offsetof(trampoline_data_t, fjmp64_seg)  == 16);
_Static_assert(offsetof(trampoline_data_t, data_ptr)    == 24);
_Static_assert(offsetof(trampoline_data_t, cr3)         == 32);
_Static_assert(offsetof(trampoline_data_t, config)      == 36);

_Static_assert(offsetof(global_ap_data_t, internal_core_id_counter) == 0);
_Static_assert(offsetof(global_ap_data_t, per_ap_data_ptrs) == 64);

extern uint8_t ap_trampoline[];
extern uint64_t ap_trampoline_gdt[];
extern uint8_t ap_trampoline32[];
extern uint8_t ap_trampoline64[];
extern trampoline_data_t ap_trampoline_data;
