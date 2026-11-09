#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace klib::pmap {
    enum Perm : uint32_t {
        R      = (1 << 2),
        W      = (1 << 3),
        X      = (1 << 4),
        U      = (1 << 5),
        G      = (1 << 6),
        UC     = (1 << 7),
        WC     = (1 << 8),
        
        URO    = U | R,
        URW    = U | R | W,
        URX    = U | R | X,
        URWX   = U | R | W | X,
        URW_UC = U | R | W | UC,
        URW_WC = U | R | W | WC,
    };

    constexpr Perm operator|(Perm a, Perm b) {
        return static_cast<Perm>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr Perm operator&(Perm a, Perm b) {
        return static_cast<Perm>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    constexpr Perm& operator|=(Perm& a, Perm b) {
        return a = a | b;
    }

    constexpr Perm& operator&=(Perm& a, Perm b) {
        return a = a & b;
    }

    std::optional<std::span<uint8_t>> map_remote(uint64_t remote_addr, size_t size, Perm perm, uint64_t addrspace_handle);
    std::optional<std::span<uint8_t>> map(uint64_t paddr, size_t size, Perm perm);

    void unmap(std::span<const uint8_t> mem);
}
