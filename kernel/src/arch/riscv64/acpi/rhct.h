#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>
#include <stdbool.h>

#define RHCT_SDT_SIGNATURE "RHCT"

struct [[gnu::packed]] RHCT {
    struct SDTHeader header;
    uint32_t flags;
    uint64_t timebase_freq;
    uint32_t node_count;
    uint32_t node_array_offset;
};

typedef enum : uint16_t {
    RHCT_TYPE_ISA_STRING = 0,
    RHCT_TYPE_CMO        = 1,
    RHCT_TYPE_MMU        = 2,
    RHCT_TYPE_HART       = 0xFFFF,
} RHCTtype;

struct [[gnu::packed]] RHCTEntryHeader {
    RHCTtype type;
    uint16_t length;
    uint16_t revision;
};

struct [[gnu::packed]] RHCT_isa_string {
    struct RHCTEntryHeader header;
    uint16_t strlen;
    char string[];
};

struct [[gnu::packed]] RHCT_cmo {
    struct RHCTEntryHeader header;
    uint8_t reserved;
    uint8_t cbom_block_size;
    uint8_t cbop_block_size;
    uint8_t cboz_block_size;
};

struct [[gnu::packed]] RHCT_mmu {
    struct RHCTEntryHeader header;
    uint8_t reserved;
    uint8_t mmu_type;
};

struct [[gnu::packed]] RHCT_hart {
    struct RHCTEntryHeader header;
    uint16_t noffsets;
    uint32_t acpi_uid;
    uint32_t offsets[];
};

#define RHCT_FOR_EACH_ENTRY(rhct, entry) for ((entry) = rhct_first_entry((struct RHCT*)(rhct)); (entry) != NULL; (entry) = rhct_next_entry((struct RHCT*)(rhct), (entry)))
extern struct RHCTEntryHeader* rhct_first_entry(struct RHCT* rhct);
extern struct RHCTEntryHeader* rhct_next_entry(struct RHCT* rhct, struct RHCTEntryHeader* entry);

extern bool acpi_parse_rhct();

typedef struct {
    char* isa_string;
    uint8_t mmu_type;
} rhct_hart_info_t;

extern rhct_hart_info_t rhct_get_hart_info(uint32_t acpi_uid);
