#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct cpuid_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_regs_t;

typedef struct cpuid_request {
    uint32_t leaf;
    uint32_t subleaf;
    char reg;
    uint32_t bit;
} cpuid_request_t;

static inline cpuid_regs_t cpuid(uint32_t leaf, uint32_t subleaf) {
    cpuid_regs_t regs;
    asm volatile (
        "cpuid"
        : "=a" (regs.eax),
          "=b" (regs.ebx),
          "=c" (regs.ecx),
          "=d" (regs.edx)
        : "a" (leaf), "c" (subleaf)
    );
    return regs;
}

enum CpuVendor {
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
    CPU_VENDOR_VIA,
    CPU_VENDOR_HYGON,
    CPU_VENDOR_ZHAOXIN,
    CPU_VENDOR_OTHER
};

enum HypervisorType {
    HYPERVISOR_TYPE_NONE,
    HYPERVISOR_TYPE_KVM,
    HYPERVISOR_TYPE_XEN,
    HYPERVISOR_TYPE_BHYVE,
    HYPERVISOR_TYPE_TCG,
    HYPERVISOR_TYPE_VBOX,
    HYPERVISOR_TYPE_VMWARE,
    HYPERVISOR_TYPE_HYPERV,
    HYPERVISOR_TYPE_OTHER,
};

#define CPUID_EAX               'a'
#define CPUID_EBX               'b'
#define CPUID_ECX               'c'
#define CPUID_EDX               'd'

#define CPUID_NO_SUBLEAF        0
#define CPUID_SUBLEAF_1         1

#define CPUID_GET_MAX_STANDARD  0x00000000
#define CPUID_GET_FEATURES      0x00000001
#define CPUID_GET_FEATURES_EXT  0x00000007
#define CPUID_GET_FREQ_INFO1    0x00000015
#define CPUID_GET_FREQ_INFO2    0x00000016
#define CPUID_GET_MAX_EXTENDED  0x80000000
#define CPUID_GET_EXT_FEATURES  0x80000001
#define CPUID_GET_CAPABILITIES  0x80000007

#define CPUID_1GB_PAGES         26  /* EDX */
#define CPUID_NX                20  /* EDX */
#define CPUID_x2APIC            21  /* ECX */
#define CPUID_ADJUST_TSC        1   /* EBX */
#define CPUID_INVARIANT_TSC     8   /* EDX */
#define CPUID_LAPIC_TSC         24  /* ECX */
#define CPUID_HYPERVISOR        31  /* ECX */
#define CPUID_SMEP              7   /* EBX */
#define CPUID_SMAP              20  /* EBX */
#define CPUID_MCE               7   /* EDX */
#define CPUID_MCA               14  /* EDX */
#define CPUID_FRED              17  /* EAX, subleaf 1 */

#define CPUID_HAS_1GB_PAGES     ((cpuid_request_t){ CPUID_GET_EXT_FEATURES, CPUID_NO_SUBLEAF, CPUID_EDX, CPUID_1GB_PAGES     })
#define CPUID_HAS_NX            ((cpuid_request_t){ CPUID_GET_EXT_FEATURES, CPUID_NO_SUBLEAF, CPUID_EDX, CPUID_NX            })
#define CPUID_HAS_x2APIC        ((cpuid_request_t){ CPUID_GET_FEATURES,     CPUID_NO_SUBLEAF, CPUID_ECX, CPUID_x2APIC        })
#define CPUID_HAS_LAPIC_TSC     ((cpuid_request_t){ CPUID_GET_FEATURES,     CPUID_NO_SUBLEAF, CPUID_ECX, CPUID_LAPIC_TSC     })
#define CPUID_HAS_ADJUST_TSC    ((cpuid_request_t){ CPUID_GET_CAPABILITIES, CPUID_NO_SUBLEAF, CPUID_EBX, CPUID_ADJUST_TSC    })
#define CPUID_HAS_INVARIANT_TSC ((cpuid_request_t){ CPUID_GET_CAPABILITIES, CPUID_NO_SUBLEAF, CPUID_EDX, CPUID_INVARIANT_TSC })
#define CPUID_HAS_HYPERVISOR    ((cpuid_request_t){ CPUID_GET_FEATURES,     CPUID_NO_SUBLEAF, CPUID_ECX, CPUID_HYPERVISOR    })
#define CPUID_HAS_SMEP          ((cpuid_request_t){ CPUID_GET_FEATURES_EXT, CPUID_NO_SUBLEAF, CPUID_EBX, CPUID_SMEP          })
#define CPUID_HAS_SMAP          ((cpuid_request_t){ CPUID_GET_FEATURES_EXT, CPUID_NO_SUBLEAF, CPUID_EBX, CPUID_SMAP          })
#define CPUID_HAS_MCE           ((cpuid_request_t){ CPUID_GET_FEATURES,     CPUID_NO_SUBLEAF, CPUID_EDX, CPUID_MCE           })
#define CPUID_HAS_MCA           ((cpuid_request_t){ CPUID_GET_FEATURES,     CPUID_NO_SUBLEAF, CPUID_EDX, CPUID_MCA           })
#define CPUID_HAS_FRED          ((cpuid_request_t){ CPUID_GET_FEATURES_EXT, CPUID_SUBLEAF_1,  CPUID_EAX, CPUID_FRED          })


// hypervisor specific CPUID bits
#define KVM_CPUID_FEATURES       0x40000001
#define KVM_CPUID_PVCLOCK        3   /* EAX */
#define KVM_CPUID_HAS_PVCLOCK    ((cpuid_request_t){ KVM_CPUID_FEATURES, CPUID_NO_SUBLEAF, CPUID_EAX, KVM_CPUID_PVCLOCK })

#define HYPERV_CPUID_INFO        0x40000001
#define HYPERV_CPUID_FEATURES    0x40000003
#define HYPERV_CPUID_PVCLOCK     9   /* EAX */
#define HYPERV_CPUID_HAS_PVCLOCK ((cpuid_request_t){ HYPERV_CPUID_FEATURES, CPUID_NO_SUBLEAF, CPUID_EAX, HYPERV_CPUID_PVCLOCK })

extern bool cpuid_check(cpuid_request_t req);
extern void get_cpu_vendor(char buf[static 13]);
extern bool get_cpu_name(char buf[static 49]);
extern bool get_hypervisor_id(char buf[static 13]);
extern void parse_cpuid();

extern bool is_hypervisor;
extern enum CpuVendor cpu_vendor;
extern enum HypervisorType hypervisor_type;
