#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <misc/font8x8_basic.h>

#include <utils/globals.h>
#include <drivers/x86_64/idt.h>
#include <drivers/x86_64/gdt.h>
#include <drivers/x86_64/pic.h>
#include <drivers/x86_64/pit.h>
#include <drivers/x86_64/ps2.h>
#include <drivers/tty.h>
#include <drivers/fb_renderer.h>
#include <interupts/interupts.h>

#include <memory/pmm.h>
#include <memory/vmm.h>
#include <memory/heap.h>

#include <filesystem/filesystem.h>
#include <filesystem/tarfs/tarfs.h>
#include <scheduler/scheduler.h>
#include <scheduler/switch.h>
#include <apps/shell.h>

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

    ft_ctx = flanterm_fb_init(
        NULL, NULL,
        framebuffer_request.response->framebuffers[0]->address,
        framebuffer_request.response->framebuffers[0]->width,
        framebuffer_request.response->framebuffers[0]->height,
        framebuffer_request.response->framebuffers[0]->pitch,
        framebuffer_request.response->framebuffers[0]->red_mask_size,
        framebuffer_request.response->framebuffers[0]->red_mask_shift,
        framebuffer_request.response->framebuffers[0]->green_mask_size,
        framebuffer_request.response->framebuffers[0]->green_mask_shift,
        framebuffer_request.response->framebuffers[0]->blue_mask_size,
        framebuffer_request.response->framebuffers[0]->blue_mask_shift,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, (void*)font8x8_basic_ft,
        8, 8, 1, 0, 0, 0
    );

    printf("Kernel: Kernel Started\n");

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

    setup_pic(0x20, 0x28);
    printf("Kernel: PIC Setup\n");

    setup_pit(1000);
    printf("Kernel: PIT Setup\n");

    setup_pmm();
    printf("Kernel: Physical Memory Manager Setup\n");

    setup_vmm();
    printf("Kernel: Virtual Memory Manager\n");

    setup_heap();
    printf("Kernel: Heap Setup\n");

    setup_tty();
    printf("Kernel: TTY Init\n");

    setup_ps2();
    printf("Kernel: PS/2 Keyboard Setup\n");

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
