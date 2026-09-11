#include <loader/elf_structs.h>
#include <string.h>

const unsigned char elf_magic[] = { 0x7F, 'E', 'L', 'F' };

bool verify_elf(void* file) {
    struct elf_header_64* header = (struct elf_header_64*)file;

    if (memcmp(header->magic, elf_magic, 4) != 0)
        return false;
    if (header->header_version != ELF_HEADER_VER)
        return false;
    if (header->elf_version != ELF_VER)
        return false;
    if (header->bits != ELF_64_BIT)
        return false;
    if (header->endianness != ELF_LITTLE_ENDIAN)
        return false;
    if (header->abi != ELF_SYSV_ABI)
        return false;
    if (header->type != ELF_EXECUTABLE_TYPE)
        return false;

#ifdef __x86_64__
    if (header->arch != ELF_X86_64_ARCH)
        return false;
#elif defined(__riscv)
    if (header->arch != ELF_RISCV_ARCH)
        return false;
#elif defined(__loongarch__)
    if (header->arch != ELF_LOONGARCH_ARCH)
        return false;
#else
    _Static_assert(false, "architecture elf type not defined");
#endif

    return true;
}
