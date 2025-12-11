#include "stddef.h"
#include "utils/panic.h"
#include <stdint.h>

#include <interupts/pit.h>
#include <syscalls/syscalls.h>
#include <memory/vmm.h>
#include <drivers/x86_64/pit.h>
#include <drivers/x86_64/pcskpr.h>
#include <drivers/x86_64/msr.h>
#include <drivers/x86_64/cpuid.h>
#include <scheduler/scheduler.h>
#include <stdio.h>

void init_syscall() {
    // enable syscall instruction
    uint64_t efer = rdmsr(EFER);
    efer |= (1 <<  0);
    wrmsr(EFER, efer);

    uint64_t star = ((uint64_t)0x18 < 48) | ((uint64_t)0x08 << 32);
    wrmsr(STAR, star);

    wrmsr(LSTAR, (uint64_t)syscall_handler);
    wrmsr(SFMASK, ~0x2);
}

void execute_syscall(struct syscall_frame* frame) {
    printf("Syscall ID %ld called from: %lx\n", frame->rax, frame->rcx);

    switch (frame->rax) {
        // get thread id
        case 0:
            frame->rax =get_current_thread()->threadId;
            break;
        // HACK: THIS IS REALLY UNSAFE
        // print (set to panic for debugging)
        case 1:
            printf("Syscall called from: %lx\n", frame->rcx);
            panic("I should not be being called", NULL);
            // printf("Printing %s\n", (char*)frame->rbx);
            frame->rax = 0;
            break;
        // play sound
        case 10:
            play_sound(frame->rbx);
            frame->rax = 0;
            break;
        // stop sound
        case 11:
            stop_sound();
            frame->rax = 0;
            break;
        // get pit cycles
        case 20:
            setup_pit(frame->rbx);
            frame->rax = 0;
            break;
        // reset pit cycles
        case 21:
            pitInteruptsTriggered = 0;
            frame->rax = 0;
            break;
        // get pit cycles
        case 22:
            frame->rax = pitInteruptsTriggered;
            break;
        default:
            frame->rax = 0xDEADBEEF;
            break;
        // map framebuffer to 0x00000000A0000000 as write combining
        case 30:
            asm volatile ("nop");
            uint32_t total_bytes = framebuffer->pitch * framebuffer->height;
            uint16_t pages = (total_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
            uint64_t virt_addr = 0x00000000A0000000;
            uint64_t phys_addr = ((uint64_t)framebuffer->address - hhdm_request.response->offset);
            for (uint16_t i = 0; i < pages; i++) {
                if (!cpu_feature_bit(1, 0, 'c', CPUID_HYPERVISOR)) {
                    // enable WC on real hardware only
                    vmm_map_page(get_current_thread()->pagemap, virt_addr, phys_addr, PTE_PRESENT | PTE_USER | PTE_WRITABLE | PTE_PCD | PTE_PAT);
                } else {
                    vmm_map_page(get_current_thread()->pagemap, virt_addr, phys_addr, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
                }
                virt_addr += 0x1000;
                phys_addr += 0x1000;
            }
            frame->rax = 0;
            break;
        // get framebuffer pitch
        case 31:
            frame->rax = framebuffer->pitch;
            break;
    }
}
