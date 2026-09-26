#pragma once
#include <sched/scheduler.h>
#include <utils/locks/irqlock.h>
#include <utils/dstruct/llist.h>

typedef struct {
    irqlock_t lock;
    llist_t threads;
} waitable_t;

typedef struct {
    llist_node_t linkage;
    thread_t* thread;
} waitable_token_t;

typedef bool (*wait_cond_t)(void* ctx);

typedef enum : int {
    WAIT_STATUS_WOKEN,
    WAIT_STATUS_TIMED_OUT,
    WAIT_STATUS_CONDITION_MET,
} wait_status_t;

extern wait_status_t sched_wait_on(waitable_t* waitable_object, uint64_t timeout);
extern wait_status_t wait_on_cond(waitable_t* waitable_object, wait_cond_t cond, void* ctx, uint64_t timeout);
extern int sched_wakeup_n_threads(waitable_t* waitable_object, int threads);
extern void sched_handle_timeouts();
