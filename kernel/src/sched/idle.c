#include "utils/atomic.h"
#include "utils/lib.h"
#include <arch/intrin/interrupts.h>
#include <stdint.h>

uint32_t cpus_started = 0;

void idle_thread_entry() {
    atomic_fetch_add32(&cpus_started, 1, AMO_RELEASE);
    enable_interrupts();
    while (true) {
        wfi();
    }
    UNREACHABLE();
}
