#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <arch/generic/cpu/halt.h>
#include <arch/generic/panic.h>
#include <arch/generic/init.h>
#include <mem/memmap.h>
#include <mem/pmm.h>
#include <mem/freelist_pmm.h>
#include <utils/misc/build_id.h>
#include <utils/limine.h>

void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();

    if (framebuffer_request.response != NULL && framebuffer_request.response->framebuffer_count >= 1)
        stdio_init(framebuffer_request.response->framebuffers[0]);
    else
        stdio_init(NULL);

    printf("EvalynOS Started\n");
    print_build_info();

    arch_early_init();

    memmap_print();
    freelist_pmm_init();

    #ifdef __x86_64__
    while (1) {
        asm volatile ("int $0xfa");
    }
    #endif

    printf("[KERNEL] Nothing to do; Halting");

    hcf();
}
