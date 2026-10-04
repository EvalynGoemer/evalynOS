#include "utils/lib.h"
#include <arch/intrin/interrupts.h>

void idle_thread_entry() {
    enable_interrupts();
    while (true) {
        wfi();
    }
    UNREACHABLE();
}
