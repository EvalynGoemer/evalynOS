#include "hardware/pit.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define LIMINE_API_REVISION 3
#include <limine.h>

#include "libc/stdio.h"
#include "libc/string.h"
#include "libc/stdlib.h"
#include "hardware/hardware.h"
#include "interupts/interupts.h"
#include "renderer/fb_renderer.h"
#include "memory/memory.h"
#include "filesystem/filesystem.h"
#include "filesystem/devfs/devfs.h"
#include "filesystem/tarfs/tarfs.h"
#include "scheduler/scheduler.h"
#include "scheduler/switch.h"

#include "apps/shell.h"

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

__attribute__((used, section(".limine_requests")))
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 3
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        __asm__ __volatile__("hlt");
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        __asm__ __volatile__("hlt");
    }

    framebuffer = framebuffer_request.response->framebuffers[0];
    FB_WIDTH = framebuffer_request.response->framebuffers[0]->width;
    FB_HEIGHT = framebuffer_request.response->framebuffers[0]->height;

    // Limine boilerplate end

    printf("Kernel: Kernel Started\n");
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

    printf("Kernel: Basic GDT & IDT Setup\n");
    printString("genericInteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 56);

    setup_pic(0x20, 0x28);
    printf("Kernel: PIC Setup\n");

    setup_pit(1000);
    printf("Kernel: PIT Setup\n");
    printString("pitInteruptsTriggered: [Logging Stripped For Preformance]", 10, FB_HEIGHT - 48);

    setup_pmm();
    printf("Kernel: Physical Memory Manager Setup\n");

    setup_vmm();
    printMemoryMap();
    printf("Kernel: Virtual Memory Manager \n");

    setup_heap();
    printf("Kernel: Heap Setup\n");

    init_devfs();
    printf("Kernel: devFS Mounted\n");

    setup_ps2();
    printf("Kernel: PS/2 Keyboard Setup\n");
    printString("ps2InteruptsTriggered: [Not Updated]", 10, FB_HEIGHT - 40);
    printString("PS/2 Keyboard Event: [Not Updated]", 10, FB_HEIGHT - 32);

    int status = init_tarfs();
    if(status == -1) {
        printf("Kernel: Could not find initramfs.tar; HALTING");
        while (1) {
            __asm__ __volatile__("cli");
            __asm__ __volatile__("hlt");
        }
    }
    printf("Kernel: tarFS as initramfs Mounted\n");

    char readBuf[512];
    status = fs_read("/test.txt", readBuf, 512);
    printf("Kernel: Printing \"test.txt\" from initramfs: %s", readBuf);

    setup_threading();
    printf("Kernel: Threading Setup\n");

    printf("Kernel: Press Enter to Start the Builtin Kernel Test CLI\n");
    int enterPressed = 0;
    while (!enterPressed) {
        char keyPressed[1];
        fs_read("/dev/ps2/kbd", keyPressed, 1);
        if (keyPressed[0] == '\n') {
            enterPressed = 1;
        }
        pit_sleep_ms(1);
    }

    create_thread(start_shell);

    while (1) {
        asm("hlt");
    }
}
