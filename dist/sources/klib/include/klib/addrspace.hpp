#pragma once

#include <cstdint>
#include <optional>
#include <klib/pmap.hpp>

namespace klib::addrspace {
    std::optional<uint64_t> create();
    void start(size_t ip, size_t sp, uint64_t addrspace_handle);
}
