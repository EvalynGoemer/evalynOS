#pragma once

#include <cstdint>
#include <span>
#include <optional>
#include <klib/pmap.hpp>

namespace klib::anon {
    extern std::optional<std::span<uint8_t>> alloc(size_t size);
    extern std::optional<size_t> alloc_remote(size_t size, size_t addr, pmap::Perm attr, uint64_t addrspace_handle);
    extern std::optional<size_t> alloc_remote(size_t size, pmap::Perm attr, uint64_t addrspace_handle);
    extern std::optional<size_t> alloc_remote(size_t size, uint64_t addr, uint64_t addrspace_handle);
    extern std::optional<size_t> alloc_remote(size_t size, uint64_t addrspace_handle);

    extern void free(std::span<uint8_t> mem);
    extern void free_remote(size_t addr, size_t len, uint64_t addrspace_handle);

    class remoteMemory {
        explicit remoteMemory(std::span<uint8_t> l, uint64_t r, uint64_t h) noexcept : localMapping(l), remote_addr(r), handle(h) {}

        std::span<uint8_t> localMapping{};
        uint64_t remote_addr;
        uint64_t handle;

        void release() noexcept;
    public:
        remoteMemory(const remoteMemory&) = delete;
        remoteMemory& operator=(const remoteMemory&) = delete;
        remoteMemory(remoteMemory&& other) noexcept : localMapping(std::exchange(other.localMapping, {})), remote_addr(other.remote_addr), handle(other.handle) {}

        remoteMemory& operator=(remoteMemory&& other) noexcept {
            if (this != &other) {
                release();
                localMapping = std::exchange(other.localMapping, {});
                remote_addr = other.remote_addr;
                handle = other.handle;
            }
            return *this;
        }

        operator std::span<uint8_t>() const noexcept { return localMapping; }
        std::span<uint8_t> span() const noexcept { return localMapping; }
        uint64_t address() const noexcept { return remote_addr; }

        static std::optional<remoteMemory> create(size_t size, size_t addr, pmap::Perm attr, uint64_t addrspace_handle);
        static std::optional<remoteMemory> create(size_t size, pmap::Perm attr, uint64_t addrspace_handle);
        static std::optional<remoteMemory> create(size_t size, size_t addr, uint64_t addrspace_handle);
        static std::optional<remoteMemory> create(size_t size, uint64_t addrspace_handle);

        void free();

        ~remoteMemory() noexcept { release(); }
    };
}
