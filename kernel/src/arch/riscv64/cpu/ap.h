#pragma once
#include "sched/scheduler.h"
#include <utils/dstruct/llist.h>
#include <utils/dstruct/stack.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    llist_node_t node;
    uint32_t hart_id;
} detected_hart_t;

extern llist_t detected_harts;

typedef struct {
    uint64_t tp;
    uint32_t id;
    thread_t* init_thread;
    uint64_t idle_thread_stack;
} per_ap_data_t;

typedef struct {
    const uint64_t ap_entry;
    uint64_t satp;
} ap_trampoline_data_t;

extern uint8_t ap_trampoline[];
extern ap_trampoline_data_t ap_trampoline_data;

// static asserts for ap.S
static_assert(offsetof(per_ap_data_t, tp) ==  0);
static_assert(offsetof(per_ap_data_t, id) ==  8);

static_assert(offsetof(ap_trampoline_data_t, ap_entry) ==  0);
static_assert(offsetof(ap_trampoline_data_t, satp) ==  8);
