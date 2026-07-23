#include <acpi/acpi.h>
#include <arch/loongarch64/acpi/madt.h>

bool setup_acpi() {
    if (!verify_acpi())
        return false;

    acpi_parse_madt();

    return true;
}
