#include <arch/generic/panic.h>
#include <arch/intrin/interrupts.h>
#include <arch/riscv64/cpu/interrupts.h>
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
        printf("x%-2d=0x%016llx ", i, frame->x[i]);
        if ((i + 1) % 4 == 0)
            printf("\n");
    }

    printf("\033[38;2;231;133;255mInterrupt Frame:\n");
    printf("SEPC   = 0x%016llx\n", frame->sepc);
    printf("SCAUSE = 0x%016llx\n", frame->scause);
    printf("STVAL  = 0x%016llx\n", frame->stval);
    printf("SSTATUS: %016b %016b %016b %016b\n",
           (unsigned)(frame->sstatus >> 48) & 0xFFFF,
           (unsigned)(frame->sstatus >> 32) & 0xFFFF,
           (unsigned)(frame->sstatus >> 16) & 0xFFFF,
           (unsigned) frame->sstatus        & 0xFFFF);


    panic_print_end();

    hcf();
}
