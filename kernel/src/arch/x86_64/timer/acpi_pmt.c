#include <arch/intrin/spin.h>
#include <arch/x86_64/intrin/portio.h>
#include <arch/x86_64/timer/timer.h>
#include <acpi/tables/fadt.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint64_t io_port = 0;

uint32_t acpi_pmt_get_ticks() {
    return inl(io_port) & 0xFFFFFF;
}

void timer_spin_wait_ms(int ms) {
    uint64_t ticks_needed = (ACPI_TIMER_FREQUENCY * ms + 999) / 1000;
    uint32_t start = acpi_pmt_get_ticks();

    while ((acpi_pmt_get_ticks() - start) < ticks_needed)
        spin();
}

void setup_acpi_pmt() {
    if (clean_fadt.xPMTimerBlock.address != 0) {
        assert(clean_fadt.xPMTimerBlock.address_space_id == ACPI_ADDRESS_TYPE_PORT_IO);
        io_port = clean_fadt.xPMTimerBlock.address;
    } else if (clean_fadt.pmTimerBlock != 0) {
        io_port = clean_fadt.pmTimerBlock;
    }

    // on x86-64 there is always an acpi pm timer present
    // and it is always used with port I/O
    assert(io_port < 0xFFFF + 1);
    assert(io_port != 0);

    LOG_TAGGED("TIME", ANSI_BBLUE, "ACPI PM Timer found at I/O Port 0x%04x", io_port);
}
