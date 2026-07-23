#include <libfdt.h>

#include <arch/riscv64/fdt/cpus.h>
#include <stdio.h>
#include <utils/limine.h>
#include <utils/lib.h>
#include <arch/generic/panic.h>

static void fdt_parse_info(void* fdt) {
    int len;
    const char *model = fdt_getprop(fdt, 0, "model", &len);
    if (model) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Board: %s", model);
    } else {
        const char *compat = fdt_getprop(fdt, 0, "compatible", &len);
        if (compat) {
            LOG_TAGGED("FDT", ANSI_BMAGENTA, "Board: %s", compat);
        }
    }

    const char *serial = fdt_getprop(fdt, 0, "serial-number", &len);
    if (serial)
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Serial: %s", serial);
}

bool setup_fdt() {
    if (!dtb_request.response) {
        LOG_TAGGED("DTB", ANSI_BMAGENTA, "DTB is not present")
        return false;
    }

    void* fdt = (void*)dtb_request.response->dtb_ptr;
    if (fdt_check_full(fdt, 0xffffffff) != 0) {
        LOG_TAGGED("DTB", ANSI_BMAGENTA, "DTB is invalid")
        return false;
    }

    bool status;

    fdt_parse_info(fdt);

    status = fdt_parse_cpus(fdt);
    if (!status)
        return false;

    return true;
}
