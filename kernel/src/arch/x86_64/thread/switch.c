#include "arch/x86_64/cpu/msr.h"
#include "arch/x86_64/descriptor_tables/gdt.h"
#include "arch/x86_64/drivers/fred/fred.h"
#include "sched/scheduler.h"
#include <stdint.h>

extern void idt_switch_to_user(uint64_t new_ip, uint64_t new_stack);

void arch_switch_to_user(uint64_t new_ip, uint64_t new_stack) {
    if (fred_enabled)
        fred_switch_to_user(new_ip, new_stack);
    else
        idt_switch_to_user(new_ip, new_stack);
}

void arch_finalize_user_switch(thread_t* next) {
    if (fred_enabled)
        wrmsr(MSR_FRED_RSP0, next->kstack_top);
    else
        bsp_tss.rsp0 = next->kstack_top;
}
