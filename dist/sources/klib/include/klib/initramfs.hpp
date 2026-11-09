#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <klib/pmap.hpp>

namespace klib {
    class initramfs {
        explicit initramfs(std::span<const uint8_t> data) noexcept : data(data) {}
        std::span<const uint8_t> data{};

        void release() noexcept;

    public:
        initramfs(const initramfs&) = delete;
        initramfs& operator=(const initramfs&) = delete;
        initramfs(initramfs&& other) noexcept : data(std::exchange(other.data, {})) {}

        initramfs& operator=(initramfs&& other) noexcept {
            auto tmp(std::move(other));
            std::swap(data, tmp.data);
            return *this;
        }

        operator std::span<const uint8_t>() const noexcept { return data; }
        std::span<const uint8_t> span() const noexcept { return data; }

        static std::optional<initramfs> get();
        ~initramfs() noexcept { release(); }
    };
}
