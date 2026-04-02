#include <arch/generic/panic.h>
#include <arch/generic/cpu/halt.h>
#include <arch/generic/cpu/interrupts.h>
#include <arch/loongarch64/cpu/interrupts.h>
#include <stdio.h>
#include <inttypes.h>

[[noreturn]]
void panic_interrupt(char* message, void* interrupt_frame) {
    disable_interrupts();
    panic_print_start(message);
    interrupt_frame_t* frame = interrupt_frame;

    printf("\033[38;2;175;56;255mGeneral Registers:\n");
    for (int i = 0; i < 32; i++) {
        printf("x%-2d=0x%016llx ", i, frame->x[i]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }

    panic_print_end();

    hcf();
}
