#include <cstdint>
#include <optional>
#include <klib/addrspace.hpp>
#include <klib/syscall.h>

namespace klib::addrspace {
    std::optional<uint64_t> create() {
        auto ret = klib::syscall0(SYS_NEW_ADDRSPACE);
        if (ret.error != ERR_NONE) return std::nullopt;
        return ret.retval;
    }

    void start(size_t ip, size_t sp, uint64_t addrspace_handle) {
        klib::syscall3(SYS_START_ADDRSPACE, ip, sp, addrspace_handle);
    }
}
