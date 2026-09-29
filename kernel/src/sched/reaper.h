#pragma once
#include <sched/wait.h>
#include <utils/locks/irqlock.h>
#include <utils/dstruct/llist.h>

extern void init_reaper();
extern llist_t reaper_list;
extern waitable_t reaper_waiter;
