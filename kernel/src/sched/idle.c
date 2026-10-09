#include "utils/lib.h"
#include <arch/intrin/interrupts.h>
#include <stdint.h>

uint32_t cpus_started = 0;

void idle_thread_entry() {
    __atomic_fetch_add(&cpus_started, 1, __ATOMIC_RELEASE);
    enable_interrupts();
    while (true) {
        wfi();
    }
    UNREACHABLE();
}
