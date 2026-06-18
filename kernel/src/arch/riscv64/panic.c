#include <arch/generic/panic.h>
#include <arch/intrin/interrupts.h>
#include <arch/riscv64/cpu/interrupts.h>
#include <stdio.h>

[[noreturn]]
void panic_interrupt(const char* message, void* interrupt_frame) {
    disable_interrupts();

    if (__atomic_exchange_n(&panic_flag, 1, __ATOMIC_SEQ_CST) != 0)
        hcf();

    spinlock_unlock(&stdio_spinlock, 0);

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
