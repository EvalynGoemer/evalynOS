#include <arch/x86_64/descriptor_tables/gdt.h>
#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/drivers/16550uart.h>
#include <arch/x86_64/drivers/fred/fred.h>

void arch_early_init() {
    setup_bsp_gdt();

    if (!setup_fred_bsp())
        setup_bsp_idt();

    setup_early_serial();
}
