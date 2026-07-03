#include <arch/intrin/cpulocal.h>
#include <mem/pmm.h>
#include <stdint.h>
#include <utils/lib.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/drivers/pvclock/hyperv_pvclock.h>

CPU_LOCAL static hyperv_pvclock_t* local_pvclock;

bool setup_hyperv_pvclock() {
    if (hypervisor_type != HYPERVISOR_TYPE_HYPERV)
        return false;
    if (!cpuid_check(HYPERV_CPUID_HAS_PVCLOCK))
        return false;

    uint64_t page = pmm_alloc_page();
    uint64_t pgreg = rdmsr(HYPERV_MSR_PVCLOCK_PAGE);
    pgreg &= 0xfff;
    pgreg |= page | 1;
    wrmsr(HYPERV_MSR_PVCLOCK_PAGE, pgreg);

    CPU_LOCAL_WRITE8(local_pvclock, TO_HHDM_PTR(page));
    return true;
}

uint64_t hyperv_pvclock_get_ns() {
    // TODO: disable preemption / thread migration while reading via cpu local
    hyperv_pvclock_t* pvclock = CPU_LOCAL_READ8(local_pvclock);

    uint32_t sequence_start, sequence_end;
    uint64_t tsc, scale, offset;

    do {
        sequence_start = pvclock->tsc_sequence;
        if (sequence_start == 0)
            return rdmsr(HYPERV_MSR_PVCLOCK_REG) * 100;

        tsc = __builtin_ia32_rdtsc();

        scale = pvclock->tsc_scale;
        offset = pvclock->tsc_offset;

        sequence_end = pvclock->tsc_sequence;
    } while (sequence_end != sequence_start);

    uint64_t time = ((__uint128_t)tsc * scale >> 64) + offset;
    return time * 100;
}
