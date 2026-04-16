#pragma once

#include <stdint.h>

#define ELF_32_BIT 1
#define ELF_64_BIT 2

#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN    2

#define ELF_HEADER_VER 1
#define ELF_VER        1

#define ELF_SYSV_ABI 0

#define ELF_EXECUTABLE_TYPE 0x02

#define ELF_X86_64_ARCH    0x3E
#define ELF_LOONGARCH_ARCH 0x102

struct [[gnu::packed]] elf_header_64 {
    char     magic[4];               // 0x7F E L F
    uint8_t  bits;                   // 1 = 32bit; 2 = 64bit
    uint8_t  endianness;             // 1 = little; 2 = big
    uint8_t  header_version;         // always 1
    uint8_t  abi;                    // 0 = sysv
    uint8_t  abi_extra;              // unused
    uint32_t _padding1;              // padding always 0
    uint16_t _padding2;              // padding always 0
    uint8_t  _padding3;              // padding always 0
    uint16_t type;                   // 0x02 = executable
    uint16_t arch;                   // 0x3E = x86-64
    uint32_t elf_version;            // always 1
    uint64_t entry_point;            // address to jump to
    uint64_t program_header_table;   // offset to the program header table
    uint64_t section_header_table;   // offset to the section header table
    uint32_t arch_flags;             // no flags on x86-64
    uint16_t elf_header_size;        // elf header size; 64 bytes for 64bit
    uint16_t program_header_size;    // program header entry size; 56 bytes for 64bit
    uint16_t program_header_entries; // number of program header table entries
    uint16_t section_header_size;    // section header entry size; 64 bytes for 64bit
    uint16_t section_header_entries; // number of section header table entries
    uint16_t section_name_index;     // index in the section table for section names
};

#define ELF_PROG_PT_NULL_TYPE 0x00000000
#define ELF_PROG_PT_LOAD_TYPE 0x00000001
#define ELF_PROG_PT_PHDR_TYPE 0x00000006

#define ELF_PROG_EXEC_FLAG 0x1
#define ELF_PROG_WRITE     0x2
#define ELF_PROG_READ_TYPE 0x4

struct [[gnu::packed]] elf_program_header_64 {
    uint32_t type;      // segment type
    uint32_t flags;     // segment flags
    uint64_t offset;    // offset of this segment's data in file
    uint64_t virt_addr; // virtual address to load to
    uint64_t phys_addr; // physical address to load to (ignored on most archs)
    uint64_t file_size; // size of segment in file
    uint64_t mem_size;  // size of segment in memory
    uint64_t alignment; // alignment of segment; 0 and 1 = no alignment; otherwize virt_addr = offset % alignment
};
