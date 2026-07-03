#include <arch/riscv64/cpu/cpulocal.h>
#include <arch/riscv64/cpu/interrupts.h>
#include <arch/intrin/cpulocal.h>
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

}

void arch_init_aps() {

}
