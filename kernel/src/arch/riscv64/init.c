#include <arch/riscv64/cpu/cpulocal.h>
#include <arch/riscv64/cpu/interrupts.h>
#include <arch/riscv64/fdt/fdt.h>
#include <arch/riscv64/acpi/acpi.h>
#include <arch/riscv64/timer/timer.h>
#include <arch/intrin/cpulocal.h>
#include <arch/generic/panic.h>
#include <utils/limine.h>
#include <stdio.h>

void arch_bootstrap_init() {
    setup_cpulocal_bsp();
}

void arch_earlycon_init() {
    if (framebuffer_request.response != NULL && framebuffer_request.response->framebuffer_count >= 1)
        stdio_init(framebuffer_request.response->framebuffers[0]);
    else
        stdio_init(NULL);
}

void arch_early_init() {
    setup_interrupts();
}

void arch_post_mm_init() {
    if (!setup_acpi()) {
        if (!setup_fdt())
            panic("Failed to setup ACPI or FDT");
    }
    setup_timer();
}

void arch_init_aps() {

}
