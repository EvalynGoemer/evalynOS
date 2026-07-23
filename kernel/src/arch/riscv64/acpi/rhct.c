#include <arch/riscv64/acpi/rhct.h>
#include <arch/riscv64/timer/timer.h>
#include <stdio.h>

bool acpi_parse_rhct() {
    struct RHCT* rhct = acpi_find_sdt(RHCT_SDT_SIGNATURE);
    if (!rhct) {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "RHCT table is not present");
        return false;
    }

    timebase_frequency = rhct->timebase_freq;

    if (timebase_frequency >= 1000000) {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "Timebase: %u MHz", timebase_frequency / 1000000);
    } else {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "Timebase: %u Hz", timebase_frequency);
    }

    return true;
}
