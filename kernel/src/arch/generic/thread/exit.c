#include "utils/lib.h"
#include <arch/intrin/cpulocal.h>
#include <sched/scheduler.h>

[[noreturn]] void thread_exit() {
    thread_t* cur = CPU_LOCAL_GET_CURRENT_THREAD();
    __atomic_store_n(&cur->state, THREAD_REAPING, __ATOMIC_SEQ_CST);
    schedule();
    UNREACHABLE();
}
