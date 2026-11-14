#include <acpi/acpi.h>
#include <arch/riscv64/acpi/madt.h>
#include <arch/riscv64/acpi/rhct.h>

bool setup_acpi() {
    if (!verify_acpi())
        return false;

    if (!acpi_parse_rhct())
        return false;

    if (!acpi_parse_madt())
        return false;

    return true;
}
