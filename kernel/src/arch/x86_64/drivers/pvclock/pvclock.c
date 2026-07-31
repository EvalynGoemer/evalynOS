#include <stdint.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/drivers/pvclock/pvclock.h>
#include <arch/x86_64/drivers/pvclock/kvm_pvclock.h>
#include <arch/x86_64/drivers/pvclock/hyperv_pvclock.h>

bool setup_pvclock() {
    if (!is_hypervisor)
        return false;

    bool status;
    switch (hypervisor_type) {
        case HYPERVISOR_TYPE_KVM: status = setup_kvm_pvclock(); break;
        case HYPERVISOR_TYPE_HYPERV: status = setup_hyperv_pvclock(); break;
        default: status = false; break;
    }

    if (!status)
        return false;

    return true;
}

uint64_t pvclock_get_ns() {
    switch (hypervisor_type) {
        case HYPERVISOR_TYPE_KVM: return kvm_pvclock_get_ns();
        case HYPERVISOR_TYPE_HYPERV: return hyperv_pvclock_get_ns();
        default: return 0;
    }
}
