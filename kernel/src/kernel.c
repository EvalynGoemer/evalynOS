#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/intrin/interrupts.h>
#include <arch/generic/cpu/ap.h>
#include <arch/generic/panic.h>
#include <arch/generic/init.h>
#include <arch/generic/paging/paging.h>
#include <mem/memmap.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <mem/freelist_pmm.h>
#include <utils/misc/build_id.h>
#include <utils/limine.h>
#include <acpi/acpi.h>

void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();

    arch_bootstrap_init();
    arch_earlycon_init();

    LOG("EvalynOS Started");
    print_build_info();

    arch_early_init();

    memmap_print();
    freelist_pmm_init();

    #ifdef __x86_64__
    paging_init();
    vmem_init();
    #endif

    setup_acpi();

    arch_post_mm_init();
    arch_init_aps();

    #ifdef __x86_64__
    uint64_t loops = 0;
    LOG_TAGGED("KERNEL", ANSI_RESET, "Starting Infinite Chunked `int 0xfa` Loop...")
    while (1) {
        uint64_t start = __builtin_ia32_rdtsc();
        while ((__builtin_ia32_rdtsc() - start) < 3000000000ULL);
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        LOG("chunk %ld done", loops++)
    }
    #endif

    #ifdef __riscv
    uint64_t loops = 0;
    LOG_TAGGED("KERNEL", ANSI_RESET, "Starting Infinite Chunked `ebreak` Loop...")
    while (1) {
        uint64_t start = csrr(0xC01);
        while ((csrr(0xC01) - start) < 10000000ULL);
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        LOG("chunk %ld done", loops++)
    }
    #endif

    #ifdef __loongarch64
    uint64_t loops = 0;
    LOG_TAGGED("KERNEL", ANSI_RESET, "Starting Infinite Chunked `break 0` Loop...")
    while (1) {
        uint64_t start = ({ uint64_t v; asm volatile("rdtime.d %0, $zero" : "=r"(v)); v; });
        while (({ uint64_t v; asm volatile("rdtime.d %0, $zero" : "=r"(v)); v; }) - start < 60000000ULL);
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        LOG("chunk %ld done", loops++)
    }
    #endif

    LOG_TAGGED("KERNEL", ANSI_RESET, "Nothing to do; Halting")

    hcf();
}
