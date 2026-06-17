#include <arch/generic/panic.h>
#include <arch/intrin/interrupts.h>
#include <stdio.h>

void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function) {
    disable_interrupts();
    printf("assertion failed: %s\nfile: %s\nline: %u\nfunction: %s\n", assertion, file, line, function);
    panic("Assertion Failed");
}
