#include <assert.h>
#include <stdint.h>
#include <utils/limine.h>
#include <loader/elf_structs.h>

uint64_t elf_introspect_base() {
    uint64_t elf_base = UINT64_MAX;
    struct limine_executable_file_response* kexec = executable_file_request.response;
    struct elf_header_64* header = (struct elf_header_64*)kexec->executable_file->address;
    struct elf_program_header_64* prog_headers = (struct elf_program_header_64*)((uint8_t*)kexec->executable_file->address + header->program_header_table);
    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64* ph = &prog_headers[i];
        if (ph->type == ELF_PROG_PT_LOAD_TYPE && ph->virt_addr < elf_base)
            elf_base = ph->virt_addr;
    }
    assert(elf_base != UINT64_MAX);
    return elf_base;
}

uint64_t elf_introspect_top() {
    uint64_t elf_top = 0;
    struct limine_executable_file_response* kexec = executable_file_request.response;
    struct elf_header_64* header = (struct elf_header_64*)kexec->executable_file->address;
    struct elf_program_header_64* prog_headers = (struct elf_program_header_64*)((uint8_t*)kexec->executable_file->address + header->program_header_table);
    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64* ph = &prog_headers[i];
        uint64_t section_top = ph->virt_addr + ph->mem_size;
        if (ph->type == ELF_PROG_PT_LOAD_TYPE && section_top > elf_top)
            elf_top = section_top;
    }

    assert(elf_top != 0);
    return elf_top;
}
