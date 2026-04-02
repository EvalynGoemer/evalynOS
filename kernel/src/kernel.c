#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <arch/generic/cpu/halt.h>
#include <arch/generic/panic.h>
#include <arch/generic/init.h>
#include <utils/misc/build_id.h>
#include <utils/limine.h>

void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
        hcf();

    stdio_init(framebuffer_request.response->framebuffers[0]);

    printf("EvalynOS Started\n");
    print_build_info();

    arch_early_init();

    #ifdef __x86_64
    printf(ANSI_BWHITE "[DBG] " ANSI_RESET "Testing Exception Handling via Page Fault\n");
    volatile int *ptr = (int *)0xDEADBEEF;
    *ptr = 0xdeafbeef;
    #endif

    panic("KRNL: Nothing to do");

    hcf();
}
