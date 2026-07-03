#include <arch/loongarch64/cpu/cpulocal.h>
#include <arch/loongarch64/cpu/interrupts.h>
#include <acpi/tables/sdt.h>
#include <arch/loongarch64/cpu/dmw.h>
#include <arch/loongarch64/paging/tlb.h>
#include <acpi/tables/spcr.h>
#include <drivers/16550uart.h>
#include <arch/intrin/cpulocal.h>
#include <utils/limine.h>
#include <stdio.h>

void arch_bootstrap_init() {
    setup_dmw();
    setup_tlb_refill_handler();
    setup_cpulocal_bsp();
}

void arch_earlycon_init() {
    if (framebuffer_request.response != NULL && framebuffer_request.response->framebuffer_count >= 1)
        stdio_init(framebuffer_request.response->framebuffers[0]);
    else
        stdio_init(NULL);

    uint64_t serial_addr = 0;
    bool serial_portio = 0;
    bool detected = detect_early_serial(&serial_addr, &serial_portio);
    if (detected)
        setup_early_serial(serial_addr + HHDM_STRONG_UNCACHED_OFFSET, serial_portio);
}

void arch_early_init() {
    setup_interrupts();
}

void arch_post_mm_init() {

}

void arch_init_aps() {

}
