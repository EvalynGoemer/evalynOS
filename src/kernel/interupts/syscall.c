#include "./interupts.h"
#include "../hardware/pit.h"
#include "../hardware/pcskpr.h"
#include "../libc/stdio.h"
#include "../renderer/fb_renderer.h"
#include "../kernel.h"
#include "../memory/vmm.h"
#include "pit.h"

__attribute__((naked))
void syscall_isr(void) {
    asm volatile (
        "push %rbp\n\t"
        "mov %rsp, %rbp\n\t"
        "push %rbx\n\t"
        "push %rcx\n\t"

        "mov %rax, %rdi\n\t"
        "mov %rbx, %rsi\n\t"
        "mov %rcx, %rdx\n\t"

        "call syscall_handler\n\t"

        "pop %rcx\n\t"
        "pop %rbx\n\t"
        "leave\n\t"
        "iretq\n\t"
    );
}

long syscall_handler(long syscall_type, long a, __attribute__((unused)) long b) {
    // play sound
    if(syscall_type == 10) {
        play_sound(a);
        return 0;
    }

    // stop sound
    if(syscall_type == 11) {
        stop_sound();
        return 0;
    }

    // set pit frequency
    if(syscall_type == 20) {
        setup_pit(a);
        return 0;
    }

    // reset pit cycles
    if(syscall_type == 21) {
        pitInteruptsTriggered = 0;
        return 0;
    }

    // get pit cycles
    if(syscall_type == 22) {
        return pitInteruptsTriggered;
    }

    // map framebuffer to 0x00000000A0000000
    if(syscall_type == 30) {
        uint64_t virt_addr = 0x00000000A0000000;
        uint64_t phys_addr = ((uint64_t)framebuffer->address - hhdm_request.response->offset);
        for (int i = 0; i < 4096; i++) {
            vmm_map_page(kernel_pagemap, virt_addr, phys_addr, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
            virt_addr += 0x1000;
            phys_addr += 0x1000;
        }
        return 0;
    }

    // get framebuffer pitch
    if(syscall_type == 31) {
        return framebuffer->pitch;
    }

    return 0xDEADBEEF;
}
