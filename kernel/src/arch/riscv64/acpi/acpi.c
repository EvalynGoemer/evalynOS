#include <acpi/acpi.h>
#include <arch/riscv64/acpi/madt.h>
#include <arch/riscv64/acpi/rhct.h>

bool setup_acpi() {
    if (!verify_acpi())
        return false;

    acpi_parse_madt();

    if (!acpi_parse_rhct())
        return false;

    return true;
}
