#include <cstdint>
#include <optional>
#include <span>

#include <klib/misc_ctl.h>
#include <klib/initramfs.hpp>
#include <klib/pmap.hpp>
#include <klib/syscall.h>

namespace klib {
    void initramfs::release() noexcept {
        if (!data.empty()) {
            pmap::unmap(data);
            data = {};
        }
    }

    std::optional<initramfs> initramfs::get() {
        auto ret = syscall1(SYS_MISC_CTL, MISC_CTL_INITRAMFS);
        if (std::intptr_t(ret.error) < 0) return std::nullopt;

        auto mapped = pmap::map(ret.retval, ret.error, pmap::Perm::URO);
        if (!mapped.has_value()) return std::nullopt;

        return initramfs{mapped.value()};
    }
}
