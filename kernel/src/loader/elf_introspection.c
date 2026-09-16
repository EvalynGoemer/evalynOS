#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <utils/limine.h>
#include <loader/elf_structs.h>

uint64_t elf_introspect_base() {
    uint64_t elf_base = UINT64_MAX;
    struct limine_executable_file_response* kexec = executable_file_request.response;
    struct elf_header_64* header = kexec->executable_file->address;
    struct elf_program_header_64* prog_headers = kexec->executable_file->address + header->program_header_table;
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
    struct elf_header_64* header = kexec->executable_file->address;
    struct elf_program_header_64* prog_headers = kexec->executable_file->address + header->program_header_table;
    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64* ph = &prog_headers[i];
        uint64_t section_top = ph->virt_addr + ph->mem_size;
        if (ph->type == ELF_PROG_PT_LOAD_TYPE && section_top > elf_top)
            elf_top = section_top;
    }

    assert(elf_top != 0);
    return elf_top;
}

void* elf_introspect_section(const char* name, uint64_t* size) {
    struct limine_executable_file_response* kexec = executable_file_request.response;
    struct elf_header_64* header = kexec->executable_file->address;
    void* file = kexec->executable_file->address;
    uint64_t file_size = kexec->executable_file->size;

    // ensure everything is in bounds
    if (header->section_header_table > file_size) return nullptr;
    uint64_t section_table_size = header->section_header_entries * sizeof(struct elf_section_header_64);
    if (section_table_size > file_size - header->section_header_table) return nullptr;
    if (header->section_header_table == 0 || header->section_header_entries == 0) return nullptr;
    if (header->section_name_index >= header->section_header_entries) return nullptr;

    struct elf_section_header_64* sections = file + header->section_header_table;
    struct elf_section_header_64* shstrtab = &sections[header->section_name_index];
    if (shstrtab->offset > file_size || shstrtab->size > file_size - shstrtab->offset) return nullptr;
    const char* strtab = file + shstrtab->offset;

    for (uint16_t i = 0; i < header->section_header_entries; i++) {
        struct elf_section_header_64* sh = &sections[i];
        if (sh->name >= shstrtab->size || strcmp(strtab + sh->name, name) != 0)
            continue;

        if (size) *size = sh->size;

        if (sh->flags & ELF_SECTION_ALLOC_FLAG) {
            struct limine_executable_address_response* kaddr = executable_address_request.response;
            return (void*)(sh->virt_addr + kaddr->virtual_base - elf_introspect_base());
        }

        if (sh->offset > file_size || sh->size > file_size - sh->offset)
            return nullptr;
        return file + sh->offset;
    }

    return NULL;
}

uint64_t elf_introspect_kaslr_slide() {
    struct limine_executable_address_response* kaddr = executable_address_request.response;
    return kaddr->virtual_base - elf_introspect_base();
}
