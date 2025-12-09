#include <interupts/interupts.h>
#include <memory/vmm.h>
#include <drivers/x86_64/pit.h>
#include <drivers/x86_64/pcskpr.h>
#include <scheduler/scheduler.h>
#include <stdio.h>

long syscall_handler(long syscall_type, long a, __attribute__((unused)) long b) {
    // get thread id
    if(syscall_type == 0) {
        return get_current_thread()->threadId;
    }

    // HACK: THIS IS REALLY UNSAFE
    // print
    if(syscall_type == 1) {
        printf("Printing %s\n", (char*)a);
        return 0;
    }

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
            vmm_map_page(get_current_thread()->pagemap, virt_addr, phys_addr, PTE_PRESENT | PTE_USER | PTE_WRITABLE);
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
