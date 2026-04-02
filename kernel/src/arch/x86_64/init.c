#include <arch/x86_64/descriptor_tables/gdt.h>
#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/drivers/16550uart.h>

void arch_early_init() {
    setup_bsp_gdt();
    setup_bsp_idt();
    setup_early_serial();
}
