#include "./interupts.h"
#include "../hardware/pit.h"
#include "../libc/stdio.h"
#include "../renderer/fb_renderer.h"

__attribute__((interrupt))
__attribute__((target("general-regs-only")))
void syscall_isr(__attribute__((unused)) struct interrupt_frame* frame) {
    printf("Userspace triggered Systemcall (INT 0x69)");
    asm("sti");
    pit_sleep_ms(1000);
}
