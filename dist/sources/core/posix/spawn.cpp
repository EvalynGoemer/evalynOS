#include <posix/spawn.hpp>
#include <posix/elf.h>

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include <klib/addrspace.hpp>
#include <klib/anon.hpp>
#include <klib/misc.hpp>
#include <klib/pmap.hpp>

#include <vfs/vfs.hpp>

using namespace klib::pmap;
using namespace klib::anon;

static bool valid_elf(const elf_header_64& header) {
    const unsigned char magic[] = {0x7F, 'E', 'L', 'F'};
    if (std::memcmp(header.magic, magic, 4) != 0) return false;
    if (header.bits != ELF_64_BIT) return false;
    if (header.endianness != ELF_LITTLE_ENDIAN) return false;
    if (header.header_version != ELF_HEADER_VER) return false;
    if (header.abi != ELF_EVALYNOS_ABI) return false;
    if (header.type != ELF_EXECUTABLE_TYPE) return false;
    if (header.arch != ELF_CURRENT_ARCH) return false;
    if (header.elf_version != ELF_VER) return false;
    return true;
}

bool posix::spawn(std::string_view path) {
    if (!vfs::rootfs) return false;

    auto file = vfs::open(path, 0);
    if (!file.has_value()) return false;

    elf_header_64 header;
    auto header_read = file.value()->read(header, 0);

    if (header_read.value_or(0) != sizeof(header)) return false;
    if (!valid_elf(header)) return false;

    std::vector<elf_program_header_64> program_headers(header.program_header_entries);
    auto phdr_read = file.value()->read(program_headers, header.program_header_table);
    if (phdr_read.value_or(0) != program_headers.size() * sizeof(elf_program_header_64))
        return false;

    auto space = klib::addrspace::create();
    if (!space.has_value()) return false;

    for (const auto& ph : program_headers) {
        if (ph.type != ELF_PROG_PT_LOAD_TYPE) continue;
        if (ph.file_size > ph.mem_size) return false;

        uint64_t start = klib::align_down(ph.virt_addr, klib::PAGE_SIZE);
        uint64_t end = klib::align_up(ph.virt_addr + ph.mem_size, klib::PAGE_SIZE);

        auto perm = Perm::U | Perm::R;
        if (ph.flags & ELF_PROG_WRITE) perm |= Perm::W;
        if (ph.flags & ELF_PROG_EXEC_FLAG) perm |= Perm::X;

        auto segment = remoteMemory::create(end - start, start, perm, space.value());
        if (!segment.has_value()) return false;

        auto dest = segment.value().span();
        size_t dest_offset = ph.virt_addr - start;

        auto seg_read = file.value()->read(dest.subspan(dest_offset, ph.file_size), ph.offset);
        if (seg_read.value_or(0) != ph.file_size) return false;
    }

    auto stack = remoteMemory::create(0x20000, Perm::URW, space.value());
    if (!stack.has_value()) return false;

    auto& stack_mem = stack.value();
    uint64_t stack_pointer = stack_mem.address() + stack_mem.span().size() - (4 * sizeof(uint64_t));

    klib::addrspace::start(header.entry_point, stack_pointer, space.value());

    return true;
}
