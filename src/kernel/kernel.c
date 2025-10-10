#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

#include "libc/stdio.h"
#include "hardware/hardware.h"
#include "interupts/interupts.h"
#include "renderer/fb_renderer.h"
#include "memory/memory.h"

#include "apps/shell.h"

#ifdef SKIP_TO_DEMOS
#include "demos/demos.h"
#endif

// Limine boilerplate start
// This limine boilerplate was taken and modified from  https://codeberg.org/Limine/limine-c-template/raw/commit/c8bc5a2b93397a19272a19a6004b0eeb1e90d982/kernel/src/main.c

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
    .revision = 0,
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 3,
    .mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .max_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

volatile struct limine_framebuffer *framebuffer;

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __asm__ __volatile__("hlt");
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        __asm__ __volatile__("hlt");
    }

    framebuffer = framebuffer_request.response->framebuffers[0];

    // Limine boilerplate end

    printf("Kernel: Kernel Started");
    printString("Kernel Debug Info", 10, FB_HEIGHT - 64);

    // Enable fxsave & fxstor instructions
    __asm__ volatile (
        "mov %%cr4, %%rax\n"
        "bts $9, %%rax\n" // fxsave & fxstor bit
        "mov %%rax, %%cr4"
        :
        :
        : "rax", "memory"
    );

    setup_gdt();
    setup_idt();

    printf("Kernel: Basic GDT & IDT Setup");
    printString("genericInteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 56);

    setup_pic(0x20, 0x28);
    printf("Kernel: PIC Setup");

    setup_pit(1000);
    printf("Kernel: PIT Setup");
    printString("pitInteruptsTriggered: [Logging Stripped For Preformance]", 10, FB_HEIGHT - 48);

    setup_ps2();
    printf("Kernel: PS/2 Keyboard Setup");
    printString("ps2InteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 40);
    printString("PS/2 Keyboard Event: [Not Updated]", 10, FB_HEIGHT - 32);

    printf("Kernel: Basic Bump Allocator Setup");
    kbump_alloc_init(0x1000000);

    vmm_init();
    printf("Kernel: Virtual Memory Manager Setup");

    printMemoryMap();

    printf("Kernel: Press Enter to Start the Builtin Kernel Test CLI");

    #ifdef SKIP_TO_DEMOS
        char charBuffer[64];

        for (int i = 0; i < 64 - 1; ++i) {
            charBuffer[i] = '\0';
        }

        clearScreen(FB_WIDTH, FB_HEIGHT);
        playBadApple(charBuffer);
        __asm__ __volatile__("hlt");
    #endif

    while (ps2LastScanCode != 28) {
        __asm__ __volatile__("hlt");
    }

    ps2LastScanCode = 0;

    while (1) {
        clearScreen(FB_WIDTH, FB_HEIGHT);
        start_shell();
    }
}
