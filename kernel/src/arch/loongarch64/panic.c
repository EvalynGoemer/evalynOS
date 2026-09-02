#include <arch/generic/panic.h>
#include <arch/intrin/interrupts.h>
#include <arch/loongarch64/cpu/interrupts.h>
#include <stdio.h>
#include <utils/locks/irqlock.h>

[[noreturn]]
void panic_interrupt(const char* message, interrupt_frame_t* frame) {
    disable_interrupts();

    if (__atomic_exchange_n(&panic_flag, 1, __ATOMIC_SEQ_CST) != 0)
        hcf();

    irqlock_unlock(&stdio_spinlock, IRQLOCK_IRQS_DISABLED);

    panic_print_start(message);

    printf("\033[38;2;175;56;255mGeneral Registers:\n");
    for (int i = 0; i < 32; i++) {
        printf("r%-2d=0x%016llx ", i, frame->x[i]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }

    printf("\033[38;2;231;133;255mInterrupt Frame:\n");
    printf("ERA   = 0x%016llx\n", frame->era);
    printf("PRMD  = 0x%016llx\n", frame->prmd);
    printf("ESTAT = 0x%016llx\n", frame->estat);
    printf("BADV  = 0x%016llx\n", frame->badv);
    printf("BADI  = 0x%08lx\n", (uint32_t)frame->badi);

    panic_print_end();

    hcf();
}
