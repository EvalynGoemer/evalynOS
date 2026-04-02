#include <stdint.h>
#include <arch/generic/panic.h>

uint64_t __stack_chk_guard = 0xdeafbeef69420bad;

[[noreturn]]
void __stack_chk_fail(void) {
    panic("SSP: Stack check failed");
}
