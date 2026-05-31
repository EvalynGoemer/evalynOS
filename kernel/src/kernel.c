#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <arch/generic/cpu/halt.h>
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

    if (framebuffer_request.response != NULL && framebuffer_request.response->framebuffer_count >= 1)
        stdio_init(framebuffer_request.response->framebuffers[0]);
    else
        stdio_init(NULL);

    LOG("EvalynOS Started");
    print_build_info();

    arch_early_init();

    memmap_print();
    freelist_pmm_init();

    #ifdef __loongarch64
    setup_acpi();
    #endif

    paging_init();

    vmem_init();

    #ifdef __x86_64__
    setup_acpi();
    #endif

    // #ifdef __x86_64__
    // uint64_t loops = 0;
    // LOG_TAGGED("KERNEL", ANSI_RESET, "Starting Infinite Chunked `int 0xfa` Loop...")
    // while (1) {
    //     uint64_t start = __builtin_ia32_rdtsc();
    //     while ((__builtin_ia32_rdtsc() - start) < 3000000000ULL);
    //     asm volatile ("int $0xfa");
    //     asm volatile ("int $0xfa");
    //     asm volatile ("int $0xfa");
    //     asm volatile ("int $0xfa");
    //     asm volatile ("int $0xfa");
    //     LOG("chunk %ld done", loops++)
    // }
    // #endif

    LOG_TAGGED("KERNEL", ANSI_RESET, "Nothing to do; Halting")

    hcf();
}
