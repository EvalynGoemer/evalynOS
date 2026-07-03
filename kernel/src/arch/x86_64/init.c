#include <stdint.h>
#include <utils/limine.h>
#include <libc/stdio.h>
#include <arch/x86_64/cpu/cpulocal.h>
#include <arch/intrin/cpulocal.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/drivers/mce/mce.h>
#include <arch/x86_64/descriptor_tables/gdt.h>
#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/drivers/fred/fred.h>
#include <arch/x86_64/drivers/pvclock/pvclock.h>
#include <drivers/16550uart.h>

void arch_bootstrap_init() {
    is_hypervisor = cpuid_check(CPUID_HAS_HYPERVISOR);
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

    // MMIO cannot be used this early on
    if (detected && serial_portio) {
        setup_early_serial(serial_addr, serial_portio);
    } else {
        // TODO: make this a cmdline argumet for assuming
        // a serial port exists and where (port i/o only for x86)
        setup_early_serial(0x3F8, true);
    }
}

void arch_early_init() {
    parse_cpuid();
    setup_bsp_gdt();

    if (!setup_fred_bsp())
        setup_bsp_idt();

    setup_control_regs();

    setup_mce();
}

void arch_post_mm_init() {
    setup_lapic();

    if (is_hypervisor)
        setup_pvclock(0);
}
