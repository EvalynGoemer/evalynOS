#include <mem/pmm.h>
#include <utils/lib.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/drivers/pvclock/kvm_pvclock.h>
#include <arch/intrin/cpulocal.h>

CPU_LOCAL static kvm_pvclock_t* local_pvclock;

bool setup_kvm_pvclock() {
    if (hypervisor_type != HYPERVISOR_TYPE_KVM)
        return false;
    if (!cpuid_check(KVM_CPUID_HAS_PVCLOCK))
        return false;

    uint64_t paddr = pmm_alloc_page();
    wrmsr(KVM_MSR_PVCLOCK, paddr | 1);
    CPU_LOCAL_WRITE8(local_pvclock, TO_HHDM_PTR(paddr));
    return true;
}

uint64_t kvm_pvclock_get_ns() {
    // TODO: disable preemption / thread migration while reading via cpu local
    kvm_pvclock_t* pvclock = CPU_LOCAL_READ8(local_pvclock);

    uint32_t version;
    uint64_t tsc, delta;
    uint64_t system_time;
    uint32_t mul;
    int8_t shift;

    while (true) {
        version = pvclock->version;

        __sync_synchronize();

        system_time = pvclock->system_time;
        uint64_t tsc_base = pvclock->tsc_timestamp;
        mul = pvclock->tsc_to_system_mul;
        shift = pvclock->tsc_shift;

        __sync_synchronize();

        if (version == pvclock->version && !(version & 1)) {
            tsc = __builtin_ia32_rdtsc();
            delta = tsc - tsc_base;

            if (shift >= 0)
                delta <<= shift;
            else
                delta >>= -shift;

            uint64_t scaled = (__uint128_t)delta * mul >> 32;

            return system_time + scaled;
        }
    }
}
