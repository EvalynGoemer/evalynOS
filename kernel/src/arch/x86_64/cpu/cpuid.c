#include <arch/x86_64/cpu/cpuid.h>
#include <stdio.h>
#include <string.h>

// populated in arch_bootstrap_init
bool is_hypervisor = false;

enum CpuVendor cpu_vendor = CPU_VENDOR_OTHER;
enum HypervisorType hypervisor_type = HYPERVISOR_TYPE_NONE;

static void classify_vendor(char vendor[static 13]) {
    if (strcmp(vendor, "GenuineIntel") == 0) {
        cpu_vendor = CPU_VENDOR_INTEL;
    // some intel CPUs report as GenuineIotel
    // https://web.archive.org/web/20191207082343/http://users.atw.hu/instlatx64/GenuineIotel/GenuineIotel00306C3_Haswell_CPUID5.txt
    } else if (strcmp(vendor, "GenuineIotel") == 0) {
        cpu_vendor = CPU_VENDOR_INTEL;
    } else if (strcmp(vendor, "AuthenticAMD") == 0) {
        cpu_vendor = CPU_VENDOR_AMD;
    } else if (strcmp(vendor, "CentaurHauls") == 0) {
        cpu_vendor = CPU_VENDOR_VIA;
    } else if (strcmp(vendor, "HygonGenuine") == 0) {
        cpu_vendor = CPU_VENDOR_HYGON;
    } else if (strcmp(vendor, "  Shanghai  ") == 0) {
        cpu_vendor = CPU_VENDOR_ZHAOXIN;
    } else {
        cpu_vendor = CPU_VENDOR_OTHER;
    }
}

static void classify_hypervisor(char hv[static 13]) {
    if (strcmp(hv, "KVMKVMKVM") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_KVM;
    } else if (strcmp(hv, "XenVMMXenVMM") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_XEN;
    } else if (strcmp(hv, "TCGTCGTCGTCG") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_TCG;
    } else if (strcmp(hv, "bhyve bhyve ") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_BHYVE;
    } else if (strcmp(hv, "VBoxVBoxVBox") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_VBOX;
    } else if (strcmp(hv, "VMwareVMware") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_VMWARE;
    } else if (strcmp(hv, "Microsoft Hv") == 0) {
        hypervisor_type = HYPERVISOR_TYPE_HYPERV;
    } else if (!is_hypervisor) {
        hypervisor_type = HYPERVISOR_TYPE_NONE;
    } else {
        hypervisor_type = HYPERVISOR_TYPE_OTHER;
    }

    // fallback check for hyperv w/ WHPX
    if (hypervisor_type == HYPERVISOR_TYPE_OTHER) {
        // check the interface signature
        if (cpuid(HYPERV_CPUID_INFO, 0).eax == 0x31237648) {
            hypervisor_type = HYPERVISOR_TYPE_HYPERV;
            strcpy(hv, "Microsoft Hv");
        }
    }
}

void parse_cpuid() {
    char vendor[13];
    char name[49];
    char hv[13];
    get_cpu_vendor(vendor);
    get_cpu_name(name);
    get_hypervisor_id(hv);

    classify_vendor(vendor);
    classify_hypervisor(hv);

    if (is_hypervisor)
        LOG_TAGGED("CPUID", ANSI_BRED, "Hypervisor ID: %s", hv);

    LOG_TAGGED("CPUID", ANSI_BRED, "CPU Vendor: %s", vendor);
    LOG_TAGGED("CPUID", ANSI_BRED, "CPU Name: %s", name);
}

bool cpuid_check(cpuid_request_t req) {
    if (req.leaf >= CPUID_GET_MAX_EXTENDED) {
        cpuid_regs_t r = cpuid(CPUID_GET_MAX_EXTENDED, 0);
        if (r.eax < req.leaf) return false;
    } else if (req.leaf >= 0x40000000) {
        if (!is_hypervisor && req.leaf <= 0x4FFFFFFF) return false;
    } else {
        cpuid_regs_t r = cpuid(CPUID_GET_MAX_STANDARD, 0);
        if (r.eax < req.leaf) return false;
    }

    if (req.leaf == CPUID_GET_CAPABILITIES && req.subleaf > cpuid(CPUID_GET_CAPABILITIES, CPUID_NO_SUBLEAF).eax)
        return false;

    if (req.leaf == CPUID_GET_FEATURES_EXT && req.subleaf > cpuid(CPUID_GET_FEATURES_EXT, CPUID_NO_SUBLEAF).eax)
        return false;

    cpuid_regs_t regs = cpuid(req.leaf, req.subleaf);

    uint32_t val;
    switch (req.reg) {
        case 'a': val = regs.eax; break;
        case 'b': val = regs.ebx; break;
        case 'c': val = regs.ecx; break;
        case 'd': val = regs.edx; break;
        default:  return false;
    }
    return (val >> req.bit) & 1u;
}

void get_cpu_vendor(char buf[static 13]) {
    struct cpuid_regs r = cpuid(0, 0);
    memcpy(&buf[0], &r.ebx, 4);
    memcpy(&buf[4], &r.edx, 4);
    memcpy(&buf[8], &r.ecx, 4);
    buf[12] = '\0';
}


bool get_cpu_name(char buf[static 49]) {
    struct cpuid_regs r = cpuid(0x80000000, 0);
    if (r.eax < 0x80000004)
        return false;

    char* p = buf;
    for (uint32_t i = 0x80000002; i <= 0x80000004; i++) {
        struct cpuid_regs regs = cpuid(i, 0);
        memcpy(p, &regs.eax, 4); p += 4;
        memcpy(p, &regs.ebx, 4); p += 4;
        memcpy(p, &regs.ecx, 4); p += 4;
        memcpy(p, &regs.edx, 4); p += 4;
    }

    buf[48] = '\0';
    return true;
}

bool get_hypervisor_id(char buf[static 13]) {
    if (!cpuid_check(CPUID_HAS_HYPERVISOR))
        return false;
    struct cpuid_regs r = cpuid(0x40000000, 0);
    memcpy(&buf[0], &r.ebx, 4);
    memcpy(&buf[4], &r.ecx, 4);
    memcpy(&buf[8], &r.edx, 4);
    buf[12] = '\0';
    return true;
}
