#include "utils/atomic.h"
#include "utils/lib.h"
#include <arch/intrin/cpulocal.h>
#include <sched/scheduler.h>

[[noreturn]] void thread_exit() {
    thread_t* cur = CPU_LOCAL_GET_CURRENT_THREAD();
    atomic_store32(&cur->state, THREAD_REAPING, AMO_SEQ_CST);
    schedule();
    UNREACHABLE();
}
