#pragma once

#include <sched/wait.h>
#include <stdint.h>

typedef struct {
    waitable_t waiters;
    uint32_t locked;
} mutex_t;

extern void mutex_lock(mutex_t* mutex);
extern void mutex_unlock(mutex_t* mutex);
