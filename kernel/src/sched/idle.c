#include <utils/atomic.h>
#include <utils/lib.h>
#include <arch/intrin/interrupts.h>
#include <arch/generic/cpu/ap.h>

void idle_thread_entry() {
    atomic_fetch_add32(&started_cpus, 1, AMO_RELEASE);
    enable_interrupts();
    while (true) {
        wfi();
    }
    UNREACHABLE();
}
