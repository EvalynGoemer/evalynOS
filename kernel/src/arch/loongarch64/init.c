#include <acpi/tables/sdt.h>
#include <arch/loongarch64/cpu/dmw.h>
#include <acpi/tables/spcr.h>
#include <drivers/16550uart.h>
#include <assert.h>
#include <utils/limine.h>
#include <stdio.h>

void arch_bootstrap_init() {
    setup_dmw();
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

}

void arch_post_mm_init() {

}

void arch_init_aps() {

}
