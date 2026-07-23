#include <libfdt.h>
#include <stdint.h>
#include <stdio.h>
#include <arch/riscv64/cpu/isa.h>
#include <arch/riscv64/timer/timer.h>

static bool fdt_cpus_parse_timebase_freq(void* fdt, int cpus_node) {
    int len;
    const void* val = fdt_getprop(fdt, cpus_node, "timebase-frequency", &len);
    if (val == nullptr) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "/cpus/timebase-frequency is missing");
        return false;
    }

    switch (len) {
        case 4: timebase_frequency = fdt_read_u32(val); break;
        case 8: timebase_frequency = fdt_read_u64(val); break;
        default: {
            LOG_TAGGED("FDT", ANSI_BMAGENTA, "/cpus/timebase-frequency is malformed");
            return false;
        }
    }

    if (timebase_frequency >= 1000000) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Timebase: %u MHz", timebase_frequency / 1000000);
    } else {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Timebase: %u Hz", timebase_frequency);
    }
    return true;
}

#define CPU_NODE_MALFORMED() {LOG_TAGGED_WARN("FDT", ANSI_BMAGENTA, "Encountered malformed cpu node"); continue;}

bool fdt_parse_cpus(void* fdt) {
    bool status;
    int len;

    int cpus_node = fdt_path_offset(fdt, "/cpus");
    if (cpus_node < 0) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "No /cpus node in DTB");
        return false;
    }

    status = fdt_cpus_parse_timebase_freq(fdt, cpus_node);
    if (!status)
        return false;

    uint64_t bsp_hart = riscv_bsp_hartid_request.response->bsp_hartid;

    int harts = 0;
    bool bsp_valid = false;
    int cpu_node;
    fdt_for_each_subnode(cpu_node, fdt, cpus_node) {
        const char* dtype = fdt_getprop(fdt, cpu_node, "device_type", &len);
        if (!dtype || strncmp(dtype, "cpu", len) != 0) continue;

        // get hart id
        const fdt32_t* reg = fdt_getprop(fdt, cpu_node, "reg", &len);
        if (!reg || len != sizeof(uint32_t)) CPU_NODE_MALFORMED();
        uint32_t hart_id = fdt_read_u32(reg);

        // verify isa string
        const char *isa = fdt_getprop(fdt, cpu_node, "riscv,isa", NULL);
        if (!isa) CPU_NODE_MALFORMED();
        if (!isa_has_baseline(isa, ISA_BASELINE_RV64GC)) continue;

        // verify status
        const char *status = fdt_getprop(fdt, cpu_node, "status", &len);
        bool ok = !status || strncmp(status, "okay", len) == 0;
        if (!ok) continue;

        // verify mmu type
        const char *mmu = fdt_getprop(fdt, cpu_node, "mmu-type", NULL);
        if (!mmu) continue;

        // TODO; get more info on the harts and build a list for AP init
        // also store the mmu type on them and drop ones not matching
        // the BSP

        if (strcmp(mmu, "riscv,sv39") == 0) {
        } else if (strcmp(mmu, "riscv,sv48") == 0) {
        } else if (strcmp(mmu, "riscv,sv57") == 0) {
        } else continue;

        harts++;

        if (hart_id == bsp_hart) {
            const char *end = strchr(isa, '_');
            int base_len = end ? (int)(end - isa) : (int)strlen(isa);
            LOG_TAGGED("FDT", ANSI_BMAGENTA, "BSP Baseline ISA: %.*s", base_len, isa);
            bsp_valid = true;
        }
    }

    if (harts == 0) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Could not find any valid harts");
        return false;
    }

    if (bsp_valid == false) {
        LOG_TAGGED("FDT", ANSI_BMAGENTA, "Could not find the BSP in the FDT");
        return false;
    }

    LOG_TAGGED("FDT", ANSI_BMAGENTA, "Found %d Usable HART(s)", harts);
    return true;
}
