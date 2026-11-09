#include <cstdint>
#include <klib/pmap.hpp>
#include <klib/misc.hpp>
#include <klib/syscall.h>

namespace klib::pmap {
    std::optional<std::span<uint8_t>> map_remote(uint64_t remote_addr, size_t size, Perm perm, uint64_t addrspace_handle) {
        auto offset = remote_addr & (klib::PAGE_SIZE - 1);
        auto ret = klib::syscall4(SYS_PMAP_REMOTE_VADDR, remote_addr - offset, size + offset, perm, addrspace_handle);
        if (ret.error != ERR_NONE) return std::nullopt;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(ret.retval + offset);
        return std::span<uint8_t>(ptr, size);
    }

    std::optional<std::span<uint8_t>> map(uint64_t paddr, size_t size, Perm perm) {
        auto offset = paddr & (klib::PAGE_SIZE - 1);
        auto ret = klib::syscall3(SYS_PMAP, paddr - offset, size + offset, perm);
        if (ret.error != ERR_NONE) return std::nullopt;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(ret.retval + offset);
        return std::span<uint8_t>(ptr, size);
    }

    void unmap(std::span<const uint8_t> mem) {
        auto addr = reinterpret_cast<size_t>(mem.data());
        auto offset = addr & (klib::PAGE_SIZE - 1);
        klib::syscall2(SYS_PUNMAP, addr - offset, mem.size_bytes() + offset);
    }
}
