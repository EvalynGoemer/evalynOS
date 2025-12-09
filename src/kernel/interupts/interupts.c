#include "interupts/spurious.h"
#include "interupts/syscall.h"
#include <interupts/interupts.h>

void dispatch_interupt (struct interrupt_frame *frame) {
    switch (frame->vector) {
        case INTERRUPT_HANDLER_DOUBLE_FAULT:
            double_fault_isr(frame);
            break;
        case INTERRUPT_HANDLER_GENERAL_PROTECTION_FAULT:
            gp_fault_isr(frame);
            break;
        case INTERRUPT_HANDLER_PAGE_FAULT:
            page_fault_isr(frame);
            break;
        case INTERRUPT_HANDLER_PIT:
            pit_isr();
            break;
        case INTERRUPT_HANDLER_PS2:
            ps2_isr();
            break;
        case INTERRUPT_HANDLER_SERIAL:
            serial_isr();
            break;
        case INTERRUPT_HANDLER_SPURIOUS_PIC_1:
            spurious_isr();
            break;
        case INTERRUPT_HANDLER_SPURIOUS_PIC_2:
            spurious_isr();
            break;
        case INTERRUPT_HANDLER_SYSCALL:
            frame->rax = syscall_handler(frame->rax, frame->rbx, frame->rcx);
            break;
        default:
            generic_isr(frame);
            break;
    }
}
