#include <cstdint>
#include <elf.h>
#include <span>
#include <optional>
#include <klib/pmap.hpp>
#include <klib/syscall.h>
#include <klib/anon.hpp>

namespace klib::anon {
    std::optional<std::span<uint8_t>> alloc(size_t size) {
        auto ret = klib::syscall1(SYS_ALLOC_ANON, size);
        if (ret.error != ERR_NONE) return std::nullopt;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(ret.retval);
        return std::span<uint8_t>(ptr, size);
    }

    std::optional<size_t> alloc_remote(size_t size, size_t addr, pmap::Perm attr, uint64_t addrspace_handle) {
        auto ret = klib::syscall4(SYS_ALLOC_ANON_REMOTE, size, addr, attr, addrspace_handle);
        if (ret.error != ERR_NONE) return std::nullopt;
        return ret.retval;
    }

    std::optional<size_t> alloc_remote(size_t size, pmap::Perm attr, uint64_t addrspace_handle) {
        return alloc_remote(size, 0, attr, addrspace_handle);
    }

    std::optional<size_t> alloc_remote(size_t size, uint64_t addr, uint64_t addrspace_handle) {
        return alloc_remote(size, addr, pmap::Perm::URW, addrspace_handle);
    }

    std::optional<size_t> alloc_remote(size_t size, uint64_t addrspace_handle) {
        return alloc_remote(size, 0, pmap::Perm::URW, addrspace_handle);
    }

    void free(std::span<uint8_t> mem) {
        klib::syscall2(SYS_FREE_ANON, size_t(mem.data()), mem.size());
    }

    void free_remote(size_t addr, size_t len, uint64_t addrspace_handle) {
        klib::syscall3(SYS_FREE_ANON_REMOTE, addr, len, addrspace_handle);
    }

    std::optional<remoteMemory> remoteMemory::create(size_t size, size_t addr, pmap::Perm attr, uint64_t addrspace_handle) {
        auto remote = alloc_remote(size, addr, attr, addrspace_handle);
        if (!remote.has_value()) return std::nullopt;
        auto local = pmap::map_remote(remote.value(), size, pmap::Perm::URW, addrspace_handle);

        if (!local.has_value()) {
            free_remote(remote.value(), size, addrspace_handle);
            return std::nullopt;
        }

        return remoteMemory(local.value(), remote.value(), addrspace_handle);
    }

    std::optional<remoteMemory> remoteMemory::create(size_t size, pmap::Perm attr, uint64_t addrspace_handle) {
        return create(size, 0, attr, addrspace_handle);
    }

    std::optional<remoteMemory> remoteMemory::create(size_t size, size_t addr, uint64_t addrspace_handle) {
        return create(size, addr, pmap::Perm::URW, addrspace_handle);
    }

    std::optional<remoteMemory> remoteMemory::create(size_t size, uint64_t addrspace_handle) {
        return create(size, 0, pmap::Perm::URW, addrspace_handle);
    }

    void remoteMemory::free() {
        free_remote(remote_addr, localMapping.size(), handle);
        release();
    }

    void remoteMemory::release() noexcept {
        if (!localMapping.empty()) {
            pmap::unmap(localMapping);
            localMapping = {};
        }
    }
}
