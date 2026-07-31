#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <acpi/tables/hpet.h>
#include <acpi/tables/sdt.h>
#include <arch/generic/paging/paging.h>
#include <mem/vmem.h>
#include <arch/intrin/mmio.h>
#include <arch/x86_64/timer/timer.h>

#define HPET_CAPABILITIES 0x00
#define HPET_CONFIG       0x10
#define HPET_COUNTER      0xF0

#define HPET_CONFIG_CNF (1 << 0)

static uint64_t hpet_vbase = 0;

bool setup_hpet() {
    struct HPET* table = acpi_find_sdt(ACPI_HPET_TABLE_SIGNATURE);
    if (!table)
        return false;

    assert(table->address.address_space_id == ACPI_ADDRESS_TYPE_MMIO);
    assert(table->address.address % PAGE_SIZE == 0);

    uint64_t paddr = table->address.address;
    hpet_vbase = vmem_alloc(&kernel_vmem_allocator, PAGE_SIZE, 0);
    paging_map_page(kernel_page_table, hpet_vbase, paddr, PAGE_KRW_UC, PAGE_SIZE_NORM);

    mmio_write_offset_64(hpet_vbase, HPET_CONFIG, HPET_CONFIG_CNF);
    assert(hpet_get_frequency() != 0);

    LOG_TAGGED("TIME", ANSI_BBLUE, "HPET Timer found at address %llx", paddr);

    return true;
}

uint64_t hpet_get_frequency() {
    if (hpet_vbase == 0)
        return 0;

    uint64_t hpet_caps = mmio_read_offset_64(hpet_vbase, HPET_CAPABILITIES);
    uint64_t tick_period = hpet_caps >> 32;
    uint64_t hpet_frequency = 1000000000000000 / tick_period;
    return hpet_frequency;
}

uint32_t hpet_get_ticks() {
    return mmio_read_64(hpet_vbase + HPET_COUNTER) & 0xFFFFFFFF;
}
